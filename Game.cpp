#include "Game.h"
#include "Graphics.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"
#include "Window.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "BufferStructs.h"

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
	InitializeConstantBuffer();
	CreateStartingCameras();

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
	XMFLOAT4 red(1.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 orange(1.0f, 0.5f, 0.0f, 1.0f);
	XMFLOAT4 yellow(1.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 green(0.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 blue(0.0f, 0.0f, 1.0f, 1.0f);
	XMFLOAT4 purple(1.0f, 0.0f, 1.0f, 1.0f);
	XMFLOAT4 black(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 darkGray(0.33f, 0.33f, 0.33f, 1.0f);
	XMFLOAT4 lightGray(0.66f, 0.66f, 0.66f, 1.0f);
	XMFLOAT4 white(1.0f, 1.0f, 1.0f, 1.0f);

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
			{ XMFLOAT3(-0.25f, +0.25f, +0.0f), black }, // Top left
			{ XMFLOAT3(-0.25f, -0.25f, +0.0f), darkGray }, // Bottom left
			{ XMFLOAT3(+0.25f, +0.25f, +0.0f), lightGray }, // Top right
			{ XMFLOAT3(+0.25f, -0.25f, +0.0f), white} // Bottom right
		};

		const unsigned int squareIndexCount = 6;

		// Set up indices, which tell us which vertices to use and in which order
		unsigned int squareIndices[] = {
			0, 3, 1,
			0, 2, 3
		};

		meshes.push_back(std::make_shared<Mesh>(squareVertices, squareIndices, squareVertexCount, squareIndexCount, "Square"));
	}

	// Third mesh: Hexagon
	{
		const unsigned int hexagonVertexCount = 7;
	
		Vertex hexagonVertices[] = 
		{
			// ADD 0.5
			{ XMFLOAT3(+0.0f, +0.0f, 0.0f), white }, // Center point
			{ XMFLOAT3(+0.0f, -0.25f, 0.0f), red }, // Bottom point
			{ XMFLOAT3(-0.1f, -0.1f, 0.0f), orange }, // Bottom-left point
			{ XMFLOAT3(-0.1f, +0.1f, 0.0f), yellow }, // Top-left point
			{ XMFLOAT3(+0.0f, +0.25f, 0.0f), green }, // Top point
			{ XMFLOAT3(+0.1f, +0.1f, 0.0f), blue }, // Top-right point
			{ XMFLOAT3(+0.1f, -0.1f, 0.0f), purple } // Bottom-right point
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

	// Create 5 GameEntities, with some sharing the same Mesh
	{
		gameEntities.push_back(std::make_shared<GameEntity>(meshes[0])); // Triangle 1
		gameEntities.push_back(std::make_shared<GameEntity>(meshes[0])); // Triangle 2
		gameEntities.push_back(std::make_shared<GameEntity>(meshes[1])); // Square 1
		gameEntities.push_back(std::make_shared<GameEntity>(meshes[2])); // Hexagon 1
		gameEntities.push_back(std::make_shared<GameEntity>(meshes[2])); // Hexagon 2
	}
}


// -----------------------------------------------------------
// Initialize a buffer on the GPU that our C++ code can change
// -----------------------------------------------------------
void Game::InitializeConstantBuffer()
{
	D3D11_BUFFER_DESC constBufferDescription = {}; // Initialize to all zeros
	constBufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constBufferDescription.ByteWidth = (sizeof(VertexShaderData) + 15) / 16 * 16; // Must be a multiple of 16
	constBufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // We have to be able to write to this buffer from C++
	constBufferDescription.Usage = D3D11_USAGE_DYNAMIC; // This buffer can change

	Graphics::Device->CreateBuffer(&constBufferDescription, 0, constBuffer.GetAddressOf());
}

// -----------------------------------------------
// Create two cameras for the user to swap between
// -----------------------------------------------
void Game::CreateStartingCameras()
{
	// First camera: set back from starting scene, standard FOV
	cameras.push_back(std::make_shared<Camera>(XMFLOAT3(0.0f, 0.0f, -3.0f), Window::AspectRatio()));
	// Second camera: further back from the starting scene, narrower FOV
	cameras.push_back(std::make_shared<Camera>(XMFLOAT3(1.0f, 0.0f, -10.0f), Window::AspectRatio(), 30.0f));
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// Example input checking: Quit if the escape key is pressed
	if (Input::KeyDown(VK_ESCAPE))
		Window::Quit();

	gameEntities[0]->GetTransform()->SetTranslation(sinf(totalTime) * 0.5f, -0.7f, 0.5f); // Triangle 1
	gameEntities[1]->GetTransform()->SetTranslation(sinf(totalTime) * -0.5f, 0.7f, -0.5f); // Triangle 2
	gameEntities[1]->GetTransform()->SetPitchYawRoll(0, 0, XMConvertToRadians(180.0f)); // Triangle 2
	gameEntities[2]->GetTransform()->SetTranslation(sinf(totalTime), cosf(totalTime), 10.0f); // Square 1
	gameEntities[3]->GetTransform()->Rotate(0.0f, 0.0f, deltaTime * 0.1f); // Hexagon 1
	gameEntities[4]->GetTransform()->Rotate(0.0f, 0.0f, deltaTime * -0.1f); // Hexagon 2

	UpdateCameras(deltaTime);

	StartImGuiUpdate(deltaTime);

	BuildCustomUI(deltaTime);
}

// ------------------------------------------------
// Calls Update() on each camera that needs updated
// ------------------------------------------------
void Game::UpdateCameras(float deltaTime)
{
	// Only update one so that others don't move while they aren't being used
	cameras[currentCameraIndex]->Update(deltaTime);
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

		// ImGui provides a variable for framerate, so use that
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

		// Dropdown tree with info on each Mesh
		if (ImGui::TreeNode("Mesh Info"))
		{
			// Tree node for each Mesh's info
			for (unsigned int i = 0; i < meshes.size(); i++)
			{
				std::shared_ptr<Mesh> currentMesh = meshes[i];
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

		if (ImGui::TreeNode("Entity Info"))
		{
			for (unsigned int i = 0; i < gameEntities.size(); i++)
			{
				std::shared_ptr<GameEntity> currentEntity = gameEntities[i];
				const char* currentEntityMeshName = currentEntity->GetMesh()->meshName.c_str();
				std::shared_ptr<Transform> currentTransform = currentEntity->GetTransform();

				ImGui::PushID(i);

				if (ImGui::TreeNode("Entity: %s", currentEntityMeshName))
				{
					XMFLOAT3 currentTranslation = currentTransform->GetTranslation();
					XMFLOAT3 currentRotation = currentTransform->GetPitchYawRoll();
					XMFLOAT3 currentScale = currentTransform->GetScale();

					ImGui::DragFloat3("Position", &currentTranslation.x, 0.1f);
					ImGui::DragFloat3("Rotation", &currentRotation.x, 0.1f);
					ImGui::DragFloat3("Scale", &currentScale.x, 0.1f);

					currentTransform->SetTranslation(currentTranslation);
					currentTransform->SetPitchYawRoll(currentRotation);
					currentTransform->SetScale(currentScale);

					// Has to be done at the end of each tree node!
					ImGui::TreePop();
				}

				ImGui::PopID();
			}

			// Has to be done at the end of each tree node!
			ImGui::TreePop();
		}

		// Dropdown for active camera info
		if (ImGui::TreeNode("Active Camera"))
		{
			ImGui::Text("Camera # %i", currentCameraIndex);
			if (ImGui::Button("Previous"))
			{
				currentCameraIndex = (currentCameraIndex - 1) % cameras.size();
			}
			ImGui::SameLine();
			if (ImGui::Button("Next"))
			{
				currentCameraIndex = (currentCameraIndex + 1) % cameras.size();
			}

			XMFLOAT3 cameraPos = cameras[currentCameraIndex]->GetTranslation();
			XMFLOAT3 cameraRot = cameras[currentCameraIndex]->GetPitchYawRoll();
			float cameraFov = cameras[currentCameraIndex]->GetFovDegrees();
			ImGui::Text("Translation: (%f, %f, %f)", cameraPos.x, cameraPos.y, cameraPos.z);
			ImGui::Text("Pitch, Yaw, Roll: (%f, %f, %f)", cameraRot.x, cameraRot.y, cameraRot.z);
			ImGui::Text("FOV (Degrees): %f", cameraFov);

			// Has to be done at the end of each tree node!
			ImGui::TreePop();
		}

		// Dropdown for colorTint and offset
		if (ImGui::TreeNode("Color Tint"))
		{
			ImGui::ColorEdit4("Color Tint", &colorTint.x);

			// Has to be done at the end of each tree node!
			ImGui::TreePop();
		}

		if (ImGui::Button("Show/Hide ImGui Demo Window"))
		{
			showImGuiDemoWindow = !showImGuiDemoWindow;
		}

		// This goes last!
		ImGui::End();
	}
}


// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	FrameStart();

	// DRAW geometry
	// - These steps are generally repeated for EACH object you draw
	// - Other Direct3D calls will also be necessary to do more complex things
	DrawAllGameEntities();

	// Draw ImGui last, so it appears over everything else.
	RenderImGui();

	FrameEnd();
}


// -----------------------------------------
// Does everything needed to start the frame
// -----------------------------------------
void Game::FrameStart()
{
	// Frame START
	// - These things should happen ONCE PER FRAME
	// - At the beginning of Game::Draw() before drawing *anything*
	{
		// Clear the back buffer (erase what's on screen) and depth buffer
		Graphics::Context->ClearRenderTargetView(Graphics::BackBufferRTV.Get(), backgroundColor);
		Graphics::Context->ClearDepthStencilView(Graphics::DepthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}
}


// ------------------------------------------------
// Loops through the Meshes list and draws each one
// ------------------------------------------------
void Game::DrawAllGameEntities()
{
	for (unsigned int i = 0; i < gameEntities.size(); i++)
	{
		// Send transform and color data to the constant buffer
		SendDataToConstantBuffer(
			gameEntities[i]->GetTransform()->GetWorldMatrix(),
			cameras[currentCameraIndex]->GetProjectionMatrix(),
			cameras[currentCameraIndex]->GetViewMatrix());
		
		// Now that the shader has access to the correct world matrix, draw the entity's Mesh
		gameEntities[i]->Draw();
	}
}

// -----------------------------------------------------------------
// Sends colorTint and offset data to the constant buffer on the GPU
// -----------------------------------------------------------------
void Game::SendDataToConstantBuffer(XMFLOAT4X4 worldMatrix, XMFLOAT4X4 projectionMatrix, XMFLOAT4X4 viewMatrix)
{
	// Prepare vsData
	vsData.colorTint = colorTint;
	vsData.worldMatrix = worldMatrix;
	vsData.projectionMatrix = projectionMatrix;
	vsData.viewMatrix = viewMatrix;

	D3D11_MAPPED_SUBRESOURCE mappedBuffer = {};
	Graphics::Context->Map(constBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);

	memcpy(mappedBuffer.pData, &vsData, sizeof(vsData));

	Graphics::Context->Unmap(constBuffer.Get(), 0);

	Graphics::Context->VSSetConstantBuffers(
		0, // Which register to bind the buffer to? (b0)
		1, // How many are we setting right now?
		constBuffer.GetAddressOf()); // Array of buffers (or address of just one)
}


// ------------------------------
// Renders ImGui for Game::Draw()
// ------------------------------
void Game::RenderImGui()
{
	ImGui::Render(); // Turns this frame’s UI into renderable triangles
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData()); // Draws it to the screen
}


void Game::FrameEnd()
{
	// Frame END
	// - These should happen exactly ONCE PER FRAME
	// - At the very end of the frame (after drawing *everything*)
	{
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


// --------------------------------------------------------
// Handle resizing to match the new window size
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
	UpdateAllCameraProjectionMatrices(Window::AspectRatio());
}

// ----------------------------------------------------------------------
// When the aspect ratio of the screen is changed, send it to all cameras
// ----------------------------------------------------------------------
void Game::UpdateAllCameraProjectionMatrices(float aspectRatio)
{
	for (unsigned int i = 0; i < cameras.size(); i++)
	{
		if (cameras[i] != nullptr) { cameras[i]->UpdateProjectionMatrix(aspectRatio); }
	}
}