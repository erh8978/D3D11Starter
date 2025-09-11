#include "Game.h"
#include "Graphics.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"
#include "Window.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

#include <DirectXMath.h>
#include <memory>

// Needed for a helper function to load pre-compiled shader files
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// The constructor is called after the window and graphics API
// are initialized but before the game loop begins
// --------------------------------------------------------
Game::Game()
{
	{ // Initialize ImGui itself & platform/renderer backends
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui_ImplWin32_Init(Window::Handle());
		ImGui_ImplDX11_Init(Graphics::Device.Get(), Graphics::Context.Get());
		// Pick a style (uncomment one of these 3)
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsLight();
		//ImGui::StyleColorsClassic();
	}

	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	LoadShaders();
	CreateGeometry();

	// Set initial graphics API state
	//  - These settings persist until we change them
	//  - Some of these, like the primitive topology & input layout, probably won't change
	//  - Others, like setting shaders, will need to be moved elsewhere later
	{
		// Tell the input assembler (IA) stage of the pipeline what kind of
		// geometric primitives (points, lines or triangles) we want to draw.  
		// Essentially: "What kind of shape should the GPU draw with our vertices?"
		Graphics::Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Ensure the pipeline knows how to interpret all the numbers stored in
		// the vertex buffer. For this course, all of your vertices will probably
		// have the same layout, so we can just set this once at startup.
		Graphics::Context->IASetInputLayout(inputLayout.Get());

		// Set the active vertex and pixel shaders
		//  - Once you start applying different shaders to different objects,
		//    these calls will need to happen multiple times per frame
		Graphics::Context->VSSetShader(vertexShader.Get(), 0, 0);
		Graphics::Context->PSSetShader(pixelShader.Get(), 0, 0);
	}
}


// --------------------------------------------------------
// Clean up memory or objects created by this class
// 
// Note: Using smart pointers means there probably won't
//       be much to manually clean up here!
// --------------------------------------------------------
Game::~Game()
{
	{ // ImGui clean up
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}
}


// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files
// and also created the Input Layout that describes our 
// vertex data to the rendering pipeline. 
// - Input Layout creation is done here because it must 
//    be verified against vertex shader byte code
// - We'll have that byte code already loaded below
// --------------------------------------------------------
void Game::LoadShaders()
{
	// BLOBs (or Binary Large OBjects) for reading raw data from external files
	// - This is a simplified way of handling big chunks of external data
	// - Literally just a big array of bytes read from a file
	ID3DBlob* pixelShaderBlob;
	ID3DBlob* vertexShaderBlob;

	// Loading shaders
	//  - Visual Studio will compile our shaders at build time
	//  - They are saved as .cso (Compiled Shader Object) files
	//  - We need to load them when the application starts
	{
		// Read our compiled shader code files into blobs
		// - Essentially just "open the file and plop its contents here"
		// - Uses the custom FixPath() helper from Helpers.h to ensure relative paths
		// - Note the "L" before the string - this tells the compiler the string uses wide characters
		D3DReadFileToBlob(FixPath(L"PixelShader.cso").c_str(), &pixelShaderBlob);
		D3DReadFileToBlob(FixPath(L"VertexShader.cso").c_str(), &vertexShaderBlob);

		// Create the actual Direct3D shaders on the GPU
		Graphics::Device->CreatePixelShader(
			pixelShaderBlob->GetBufferPointer(),	// Pointer to blob's contents
			pixelShaderBlob->GetBufferSize(),		// How big is that data?
			0,										// No classes in this shader
			pixelShader.GetAddressOf());			// Address of the ID3D11PixelShader pointer

		Graphics::Device->CreateVertexShader(
			vertexShaderBlob->GetBufferPointer(),	// Get a pointer to the blob's contents
			vertexShaderBlob->GetBufferSize(),		// How big is that data?
			0,										// No classes in this shader
			vertexShader.GetAddressOf());			// The address of the ID3D11VertexShader pointer
	}

	// Create an input layout 
	//  - This describes the layout of data sent to a vertex shader
	//  - In other words, it describes how to interpret data (numbers) in a vertex buffer
	//  - Doing this NOW because it requires a vertex shader's byte code to verify against!
	//  - Luckily, we already have that loaded (the vertex shader blob above)
	{
		D3D11_INPUT_ELEMENT_DESC inputElements[2] = {};

		// Set up the first element - a position, which is 3 float values
		inputElements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;				// Most formats are described as color channels; really it just means "Three 32-bit floats"
		inputElements[0].SemanticName = "POSITION";							// This is "POSITION" - needs to match the semantics in our vertex shader input!
		inputElements[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// How far into the vertex is this?  Assume it's after the previous element

		// Set up the second element - a color, which is 4 more float values
		inputElements[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;			// 4x 32-bit floats
		inputElements[1].SemanticName = "COLOR";							// Match our vertex shader input!
		inputElements[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// After the previous element

		// Create the input layout, verifying our description against actual shader code
		Graphics::Device->CreateInputLayout(
			inputElements,							// An array of descriptions
			2,										// How many elements in that array?
			vertexShaderBlob->GetBufferPointer(),	// Pointer to the code of a shader that uses this layout
			vertexShaderBlob->GetBufferSize(),		// Size of the shader code that uses this layout
			inputLayout.GetAddressOf());			// Address of the resulting ID3D11InputLayout pointer
	}
}


// --------------------------------------------------------
// Creates the geometry we're going to draw
// --------------------------------------------------------
void Game::CreateGeometry()
{
	// Create some temporary variables to represent colors
	// - Not necessary, just makes things more readable
	XMFLOAT4 red = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 orange = XMFLOAT4(1.0f, 0.5f, 0.0f, 1.0f);
	XMFLOAT4 yellow = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 green = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 blue = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	XMFLOAT4 purple = XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f);
	XMFLOAT4 black = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 darkGray = XMFLOAT4(0.33f, 0.33f, 0.33f, 1.0f);
	XMFLOAT4 lightGray = XMFLOAT4(0.66f, 0.66f, 0.66f, 1.0f);
	XMFLOAT4 white = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	// Create three unique Meshes
	// Start with a smaller version of the original triangle.
	// - It'll be a bit smaller than it was originally,
	//   to make more room on the screen for the other triangles.
	// - Note: Since we don't have a camera or really any concept of
	//    a "3d world" yet, we're simply describing positions within the
	//    bounds of how the rasterizer sees our screen: [-1 to +1] on X and Y
	// - This means (0,0) is at the very center of the screen.
	// - These are known as "Normalized Device Coordinates" or "Homogeneous 
	//    Screen Coords", which are ways to describe a position without
	//    knowing the exact size (in pixels) of the image/window/etc.  
	// - Long story short: Resizing the window also resizes the triangle,
	//    since we're describing the triangle in terms of the window itself
	{
		const unsigned int triangleVertexCount = 3;

		Vertex triangleVertices[] = 
		{
			{ XMFLOAT3(+0.0f, +0.25f, +0.0f), red }, // Top
			{ XMFLOAT3(+0.25f, -0.25f, +0.0f), blue }, // Right
			{ XMFLOAT3(-0.25f, -0.25f, +0.0f), green }, // Left
		};

		const unsigned int triangleIndexCount = 3;

		// Set up indices, which tell us which vertices to use and in which order
		// - This is redundant for just 3 vertices, but necessary for how the Mesh class is designed
		unsigned int triangleIndices[] = {
			0, 1, 2
		};

		meshes.push_back(std::make_shared<Mesh>(triangleVertices, triangleIndices, triangleVertexCount, triangleIndexCount, "Triangle"));
	}

	// Second Mesh: Two triangles, forming a square
	{
		const unsigned int squareVertexCount = 4;

		Vertex squareVertices[] =
		{
			{ XMFLOAT3(+0.75f, +0.5f, +0.0f), black }, // Top left
			{ XMFLOAT3(+0.75f, +0.25f, +0.0f), darkGray }, // Bottom left
			{ XMFLOAT3(+0.5f, +0.5f, +0.0f), lightGray }, // Top right
			{ XMFLOAT3(+0.5f, +0.25f, +0.0f), white} // Bottom right
		};

		const unsigned int squareIndexCount = 6;

		// Set up indices, which tell us which vertices to use and in which order
		unsigned int squareIndices[] = {
			0, 1, 2,
			1, 3, 2
		};

		meshes.push_back(std::make_shared<Mesh>(squareVertices, squareIndices, squareVertexCount, squareIndexCount, "Square"));
	}

	// Third mesh: Hexagon
	{
		const unsigned int hexagonVertexCount = 7;
	
		Vertex hexagonVertices[] = 
		{
			// ADD 0.5
			{ XMFLOAT3(+0.5f, -0.5f, 0.0f), white }, // Center point
			{ XMFLOAT3(+0.5f, -0.75f, 0.0f), red }, // Bottom point
			{ XMFLOAT3(+0.4f, -0.6f, 0.0f), orange }, // Bottom-left point
			{ XMFLOAT3(+0.4f, -0.4f, 0.0f), yellow }, // Top-left point
			{ XMFLOAT3(+0.5f, -0.25f, 0.0f), green }, // Top point
			{ XMFLOAT3(+0.6f, -0.4f, 0.0f), blue }, // Top-right point
			{ XMFLOAT3(+0.6f, -0.6f, 0.0f), purple } // Bottom-right point
		};

		const unsigned int hexagonIndexCount = 18;

		// Arranged vertically for clarity
		unsigned int hexagonIndices[] = {
			0, 1, 2,
			0, 2, 3,
			0, 3, 4,
			0, 4, 5,
			0, 5, 6,
			0, 6, 1
		};

		meshes.push_back(std::make_shared<Mesh>(hexagonVertices, hexagonIndices, hexagonVertexCount, hexagonIndexCount, "Hexagon"));
	}
}


// --------------------------------------------------------
// Handle resizing to match the new window size
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
	
}


// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// Example input checking: Quit if the escape key is pressed
	if (Input::KeyDown(VK_ESCAPE))
		Window::Quit();

	StartImGuiUpdate(deltaTime);

	BuildCustomUI(deltaTime);
}


// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Frame START
	// - These things should happen ONCE PER FRAME
	// - At the beginning of Game::Draw() before drawing *anything*
	{
		// Clear the back buffer (erase what's on screen) and depth buffer
		Graphics::Context->ClearRenderTargetView(Graphics::BackBufferRTV.Get(),	backgroundColor);
		Graphics::Context->ClearDepthStencilView(Graphics::DepthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	// DRAW geometry
	// - These steps are generally repeated for EACH object you draw
	// - Other Direct3D calls will also be necessary to do more complex things
	{
		DrawAllMeshes();
	}

	// Frame END
	// - These should happen exactly ONCE PER FRAME
	// - At the very end of the frame (after drawing *everything*)
	{
		// Draw ImGui last, so it appears over everything else.
		Game::RenderImGui();

		// Present at the end of the frame
		bool vsync = Graphics::VsyncState();
		Graphics::SwapChain->Present(
			vsync ? 1 : 0,
			vsync ? 0 : DXGI_PRESENT_ALLOW_TEARING);

		// Re-bind back buffer and depth buffer after presenting
		Graphics::Context->OMSetRenderTargets(
			1,
			Graphics::BackBufferRTV.GetAddressOf(),
			Graphics::DepthBufferDSV.Get());
	}
}


// -----------------------------------------------------
// Does all basic ImGui-related tasks for Game::Update()
// -----------------------------------------------------
void Game::StartImGuiUpdate(float deltaTime)
{
	// Struct for data transfer to and from ImGui
	ImGuiIO& io = ImGui::GetIO();

	{ // Send fresh window and time data to ImGui
		io.DeltaTime = deltaTime;
		io.DisplaySize.x = (float)Window::Width();
		io.DisplaySize.y = (float)Window::Height();
	}

	{ // Reset ImGui frame
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}

	{ // Determine new input capture
		Input::SetKeyboardCapture(io.WantCaptureKeyboard);
		Input::SetMouseCapture(io.WantCaptureMouse);
	}

	// Render the ImGui demo window
	if (showImGuiDemoWindow)
	{
		ImGui::ShowDemoWindow();
	}
}


// --------------------------------
// Creates a custom ImGui interface
// --------------------------------
void Game::BuildCustomUI(float deltaTime)
{
	{ // Any ImGui methods called between ImGui::Begin() and ImGui::End() will be placed in a new window.
		ImGui::Begin("540 ImGui Window");

		// Framerate is how many frames happen in a second,
		// DeltaTime is the duration of one frame (in seconds),
		// So divide one second into DeltaTime frames to get how many frames per second.
		// float framerate = 1.0 / deltaTime;

		// ...or ImGui could just provide that info for free.
		float framerate = ImGui::GetIO().Framerate;
		ImGui::Text("Framerate: %f fps", framerate);

		int width = Window::Width();
		int height = Window::Height();
		ImGui::Text("Window Dimensions: %ix%ip", width, height);

		// Because backgroundColor is an array pointer, any changes to it automatically happen elsewhere (mostly in Draw())
		ImGui::ColorEdit4("Game Background Color", backgroundColor);

		if (ImGui::Button("Reset Background"))
		{
			// I can't figure out a better way of resetting the array's values,
			// so here I am doing it like a fool.
			backgroundColor[0] = 0.4f;
			backgroundColor[1] = 0.6f;
			backgroundColor[2] = 0.75f;
			backgroundColor[3] = 1.0f;
		}

		if (ImGui::Button("Show/Hide ImGui Demo Window"))
		{
			showImGuiDemoWindow = !showImGuiDemoWindow;
		}

		// Slider for user "rating" of debug UI
		ImGui::Text("Please rate this Debug UI.");
		static int rating = 5;
		ImGui::SliderInt("/10", &rating, 1, 10);

		// Display special text if the slider is at either extreme
		if (rating >= 10)
		{
			ImGui::Text("Thank you!!!");
		}
		else if (rating <= 1)
		{
			ImGui::Text("Awwww....");
		}

		// Text input box
		ImGui::Text("Leave your feedback below!");
		const int maximumFeedbackLength = 64;
		static char feedback[maximumFeedbackLength];
		ImGui::InputText("Feedback", feedback, maximumFeedbackLength);

		// Dropdown tree with info on each Mesh
		if (ImGui::TreeNode("Mesh Info"))
		{
			// Tree node for each Mesh's info
			for (unsigned int i = 0; i < meshes.size(); i++)
			{
				Mesh* currentMesh = meshes[i].get();
				const char* currentMeshName = currentMesh->meshName.c_str();

				ImGui::PushID(i);

				if (ImGui::TreeNode("Mesh: %s", currentMeshName))
				{
					unsigned int numVertices = currentMesh->GetVertexCount();
					unsigned int numIndices = currentMesh->GetIndexCount();
					unsigned int numTriangles = numIndices / 3;

					ImGui::Text("Triangles: %i", numTriangles);
					ImGui::Text("Vertices: %i", numVertices);
					ImGui::Text("Indicies: %i", numIndices);

					// Has to be done at the end of each tree node!
					ImGui::TreePop();
				}

				ImGui::PopID();
			}

			// Has to be done at the end of each tree node!
			ImGui::TreePop();
		}

		// This goes last!
		ImGui::End();
	}
}

// ------------------------------
// Renders ImGui for Game::Draw()
// ------------------------------
void Game::RenderImGui()
{
	ImGui::Render(); // Turns this frame’s UI into renderable triangles
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData()); // Draws it to the screen
}

// ------------------------------------------------
// Loops through the Meshes list and draws each one
// ------------------------------------------------
void Game::DrawAllMeshes()
{
	for (unsigned int i = 0; i < meshes.size(); i++)
	{
		meshes[i]->Draw();
	}
}


