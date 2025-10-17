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
#include "Material.h"

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
	CreateGameEntities();
	InitializeConstantBuffers();
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
	}

	// Create an input layout 
	//  - This describes the layout of data sent to a vertex shader
	//  - In other words, it describes how to interpret data (numbers) in a vertex buffer
	//  - Doing this NOW because it requires a vertex shader's byte code to verify against!
	//  - Luckily, we already have that loaded (the vertex shader blob above)
	{
		D3D11_INPUT_ELEMENT_DESC inputElements[3] = {};

		// Set up the first element - a position, which is 3 float values
		inputElements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;				// Most formats are described as color channels; really it just means "Three 32-bit floats"
		inputElements[0].SemanticName = "POSITION";							// This is "POSITION" - needs to match the semantics in our vertex shader input!
		inputElements[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// How far into the vertex is this?  Assume it's after the previous element

		// Set up the second element - a UV coordinate, which is 2 more float values
		inputElements[1].Format = DXGI_FORMAT_R32G32_FLOAT;					// 2x 32-bit floats
		inputElements[1].SemanticName = "TEXCOORD";							// Match our vertex shader input!
		inputElements[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// After the previous element

		// Set up the third element - a surface normal, which is 3 more float values
		inputElements[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;				// 3x 32-bit floats
		inputElements[2].SemanticName = "NORMAL";							// Once again, match the vertex shader input!
		inputElements[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;  // After the previous element

		// Create the input layout, verifying our description against actual shader code
		Graphics::Device->CreateInputLayout(
			inputElements,							// An array of descriptions
			3,										// How many elements in that array?
			vertexShaderBlob->GetBufferPointer(),	// Pointer to the code of a shader that uses this layout
			vertexShaderBlob->GetBufferSize(),		// Size of the shader code that uses this layout
			inputLayout.GetAddressOf());			// Address of the resulting ID3D11InputLayout pointer
	}
}


// --------------------------------------------------
// Loads a single vertex shader from a given filepath
// --------------------------------------------------
Microsoft::WRL::ComPtr<ID3D11VertexShader> Game::LoadVertexShader(const WCHAR* shaderPath)
{
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;

	// Load data into a BLOB (Binary Large OBject)
	ID3DBlob* vertexShaderBlob;
	D3DReadFileToBlob(FixPath(shaderPath).c_str(), &vertexShaderBlob);

	// Create the actual shader in GPU memory
	Graphics::Device->CreateVertexShader(
		vertexShaderBlob->GetBufferPointer(),	// Get a pointer to the blob's contents
		vertexShaderBlob->GetBufferSize(),		// How big is that data?
		0,										// No classes in this shader
		vertexShader.GetAddressOf());			// The address of the ID3D11VertexShader pointer

	return vertexShader;
}


// -------------------------------------------------
// Loads a single pixel shader from a given filepath
// -------------------------------------------------
Microsoft::WRL::ComPtr<ID3D11PixelShader> Game::LoadPixelShader(const WCHAR* shaderPath)
{
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;

	// Load data into a BLOB (Binary Large OBject)
	ID3DBlob* pixelShaderBlob;
	D3DReadFileToBlob(FixPath(shaderPath).c_str(), &pixelShaderBlob);

	// Create the actual shader in GPU memory
	Graphics::Device->CreatePixelShader(
		pixelShaderBlob->GetBufferPointer(),	// Pointer to blob's contents
		pixelShaderBlob->GetBufferSize(),		// How big is that data?
		0,										// No classes in this shader
		pixelShader.GetAddressOf());			// Address of the ID3D11PixelShader pointer

	return pixelShader;
}


// --------------------------------------------------------
// Creates the geometry we're going to draw
// --------------------------------------------------------
void Game::CreateGameEntities()
{
	// Load vertex & pixel shaders - just one of each for now
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader = LoadVertexShader(L"VertexShader.cso");

	Microsoft::WRL::ComPtr<ID3D11PixelShader> basicPixelShader = LoadPixelShader(L"PixelShader.cso");
	Microsoft::WRL::ComPtr<ID3D11PixelShader> debugUVsPixelShader = LoadPixelShader(L"DebugUVsPS.cso");
	Microsoft::WRL::ComPtr<ID3D11PixelShader> debugNormalsPixelShader = LoadPixelShader(L"DebugNormalsPS.cso");
	Microsoft::WRL::ComPtr<ID3D11PixelShader> customPixelShader = LoadPixelShader(L"CustomPS.cso");

	// Create some colorTints
	XMFLOAT4 blackTint(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 whiteTint(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 redTint(1.0f, 0.2f, 0.2f, 1.0f);
	XMFLOAT4 greenTint(0.2f, 1.0f, 0.2f, 1.0f);
	XMFLOAT4 blueTint(0.2f, 0.2f, 1.0f, 1.0f);

	// Material 1 - Color tint shader, red
	std::shared_ptr<Material> basicRedMaterial = std::make_shared<Material>(redTint, vertexShader, basicPixelShader);

	// Material 2 - Color tint shader, green
	std::shared_ptr<Material> basicGreenMaterial = std::make_shared<Material>(greenTint, vertexShader, basicPixelShader);;

	// Material 3 - Color tint shader, blue
	std::shared_ptr<Material> basicBlueMaterial = std::make_shared<Material>(blueTint, vertexShader, basicPixelShader);;

	// Material 4 - UVs shader
	std::shared_ptr<Material> debugUVsMaterial = std::make_shared<Material>(whiteTint, vertexShader, debugUVsPixelShader);

	// Material 5 - Normals shader
	std::shared_ptr<Material> debugNormalsMaterial = std::make_shared<Material>(whiteTint, vertexShader, debugNormalsPixelShader);

	// Material 6 - Custom shader
	std::shared_ptr<Material> customMaterial = std::make_shared<Material>(blackTint, vertexShader, customPixelShader);

	// Create a Mesh for each .obj file, and add them to meshes
	{
		meshes.push_back(std::make_shared<Mesh>(FixPath("../../Assets/Meshes/cube.obj").c_str(), "Cube")); // Cube
		meshes.push_back(std::make_shared<Mesh>(FixPath("../../Assets/Meshes/cylinder.obj").c_str(), "Cylinder")); // Cylinder
		meshes.push_back(std::make_shared<Mesh>(FixPath("../../Assets/Meshes/helix.obj").c_str(), "Helix")); // Helix
		meshes.push_back(std::make_shared<Mesh>(FixPath("../../Assets/Meshes/sphere.obj").c_str(), "Sphere")); // Sphere
		meshes.push_back(std::make_shared<Mesh>(FixPath("../../Assets/Meshes/torus.obj").c_str(), "Torus")); // Torus
		meshes.push_back(std::make_shared<Mesh>(FixPath("../../Assets/Meshes/quad.obj").c_str(), "Quad")); // Quad
		meshes.push_back(std::make_shared<Mesh>(FixPath("../../Assets/Meshes/quad_double_sided.obj").c_str(), "Double-Sided Quad")); // Double-Sided Quad
	}

	// Create 21 GameEntities, with some sharing the same Mesh
	{
		// Row 1 - debugNormalsMaterial
		gameEntities.push_back(std::make_shared<GameEntity>(meshes[0], debugNormalsMaterial)); // Cube
		gameEntities.push_back(std::make_shared<GameEntity>(meshes[1], debugNormalsMaterial)); // Cylinder
		gameEntities.push_back(std::make_shared<GameEntity>(meshes[2], debugNormalsMaterial)); // Helix
		gameEntities.push_back(std::make_shared<GameEntity>(meshes[3], debugNormalsMaterial)); // Sphere
		gameEntities.push_back(std::make_shared<GameEntity>(meshes[4], debugNormalsMaterial)); // Torus
		gameEntities.push_back(std::make_shared<GameEntity>(meshes[5], debugNormalsMaterial)); // Quad
		gameEntities.push_back(std::make_shared<GameEntity>(meshes[6], debugNormalsMaterial)); // Double-Sided Quad
		gameEntities[0]->GetTransform()->SetTranslation(-3.0f, 1.0f, 0.0f);
		gameEntities[1]->GetTransform()->SetTranslation(-2.0f, 1.0f, 0.0f);
		gameEntities[2]->GetTransform()->SetTranslation(-1.0f, 1.0f, 0.0f);
		gameEntities[3]->GetTransform()->SetTranslation(0.0f, 1.0f, 0.0f);
		gameEntities[4]->GetTransform()->SetTranslation(1.0f, 1.0f, 0.0f);
		gameEntities[5]->GetTransform()->SetTranslation(2.0f, 1.0f, 0.0f);
		gameEntities[6]->GetTransform()->SetTranslation(3.0f, 1.0f, 0.0f);

		// Row 2 - debugUVsMaterial
		gameEntities.push_back(std::make_shared<GameEntity>(meshes[0], debugUVsMaterial)); // Cube
		gameEntities.push_back(std::make_shared<GameEntity>(meshes[1], debugUVsMaterial)); // Cylinder
		gameEntities.push_back(std::make_shared<GameEntity>(meshes[2], debugUVsMaterial)); // Helix
		gameEntities.push_back(std::make_shared<GameEntity>(meshes[3], debugUVsMaterial)); // Sphere
		gameEntities.push_back(std::make_shared<GameEntity>(meshes[4], debugUVsMaterial)); // Torus
		gameEntities.push_back(std::make_shared<GameEntity>(meshes[5], debugUVsMaterial)); // Quad
		gameEntities.push_back(std::make_shared<GameEntity>(meshes[6], debugUVsMaterial)); // Double-Sided Quad
		gameEntities[7]->GetTransform()->SetTranslation(-3.0f, 0.0f, 0.0f);
		gameEntities[8]->GetTransform()->SetTranslation(-2.0f, 0.0f, 0.0f);
		gameEntities[9]->GetTransform()->SetTranslation(-1.0f, 0.0f, 0.0f);
		gameEntities[10]->GetTransform()->SetTranslation(0.0f, 0.0f, 0.0f);
		gameEntities[11]->GetTransform()->SetTranslation(1.0f, 0.0f, 0.0f);
		gameEntities[12]->GetTransform()->SetTranslation(2.0f, 0.0f, 0.0f);
		gameEntities[13]->GetTransform()->SetTranslation(3.0f, 0.0f, 0.0f);

		// Row 3 - basicMaterial and customMaterial
		gameEntities.push_back(std::make_shared<GameEntity>(meshes[0], basicRedMaterial)); // Cube
		gameEntities.push_back(std::make_shared<GameEntity>(meshes[1], basicGreenMaterial)); // Cylinder
		gameEntities.push_back(std::make_shared<GameEntity>(meshes[2], basicBlueMaterial)); // Helix
		gameEntities.push_back(std::make_shared<GameEntity>(meshes[3], customMaterial)); // Sphere
		gameEntities.push_back(std::make_shared<GameEntity>(meshes[4], basicBlueMaterial)); // Torus
		gameEntities.push_back(std::make_shared<GameEntity>(meshes[5], basicGreenMaterial)); // Quad
		gameEntities.push_back(std::make_shared<GameEntity>(meshes[6], basicRedMaterial)); // Double-Sided Quad
		gameEntities[14]->GetTransform()->SetTranslation(-3.0f, -1.0f, 0.0f);
		gameEntities[15]->GetTransform()->SetTranslation(-2.0f, -1.0f, 0.0f);
		gameEntities[16]->GetTransform()->SetTranslation(-1.0f, -1.0f, 0.0f);
		gameEntities[17]->GetTransform()->SetTranslation(0.0f, -1.0f, 0.0f);
		gameEntities[18]->GetTransform()->SetTranslation(1.0f, -1.0f, 0.0f);
		gameEntities[19]->GetTransform()->SetTranslation(2.0f, -1.0f, 0.0f);
		gameEntities[20]->GetTransform()->SetTranslation(3.0f, -1.0f, 0.0f);
	}

	// Make the entities smaller, so they aren't huge (for now)
	for (unsigned int i = 0; i < gameEntities.size(); i++)
	{
		gameEntities[i]->GetTransform()->SetScale(0.3f, 0.3f, 0.3f);
	}
}


// -----------------------------------------------------------
// Initialize a buffer on the GPU that our C++ code can change
// -----------------------------------------------------------
void Game::InitializeConstantBuffers()
{
	// Vertex Shader
	D3D11_BUFFER_DESC vsConstBufferDescription = {}; // Initialize to all zeros
	vsConstBufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	vsConstBufferDescription.ByteWidth = (sizeof(VertexShaderExternalData) + 15) / 16 * 16; // Must be a multiple of 16
	vsConstBufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // We have to be able to write to this buffer from C++
	vsConstBufferDescription.Usage = D3D11_USAGE_DYNAMIC; // This buffer can change

	Graphics::Device->CreateBuffer(&vsConstBufferDescription, 0, vertexShaderConstantBuffer.GetAddressOf());

	// Pixel Shader
	D3D11_BUFFER_DESC psConstBufferDescription = {}; // Initialize to all zeros
	psConstBufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	psConstBufferDescription.ByteWidth = (sizeof(PixelShaderExternalData) + 15) / 16 * 16; // Must be a multiple of 16
	psConstBufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // We have to be able to write to this buffer from C++
	psConstBufferDescription.Usage = D3D11_USAGE_DYNAMIC; // This buffer can change

	Graphics::Device->CreateBuffer(&psConstBufferDescription, 0, pixelShaderConstantBuffer.GetAddressOf());
}


// -----------------------------------------------
// Create two cameras for the user to swap between
// -----------------------------------------------
void Game::CreateStartingCameras()
{
	// First camera: set back from starting scene, standard FOV
	cameras.push_back(std::make_shared<Camera>(XMFLOAT3(0.0f, 0.0f, -5.0f), Window::AspectRatio()));
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

	for (unsigned int i = 0; i < gameEntities.size(); i++)
	{
		gameEntities[i]->GetTransform()->Rotate(0.0f, 1.0f * deltaTime, 0.0f);
	}

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

		ImGui::Text("Camera Controls");
		ImGui::Text("	W / S: Forwards / Backwards");
		ImGui::Text("	A / D: Left / Right");
		ImGui::Text("	Q / E: Down / Up");
		ImGui::Text("	LMB + Drag: Rotate");

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
	DrawAllGameEntities(totalTime);

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
void Game::DrawAllGameEntities(float totalTime)
{
	for (unsigned int i = 0; i < gameEntities.size(); i++)
	{
		// Set shaders from the Material
		Graphics::Context->VSSetShader(gameEntities[i]->GetMaterial()->GetVertexShader().Get(), 0, 0);
		Graphics::Context->PSSetShader(gameEntities[i]->GetMaterial()->GetPixelShader().Get(), 0, 0);

		// Send transform and color data to the constant buffer
		SendDataToConstantBuffer(
			gameEntities[i]->GetMaterial()->GetColorTint(),
			gameEntities[i]->GetTransform()->GetWorldMatrix(),
			cameras[currentCameraIndex]->GetProjectionMatrix(),
			cameras[currentCameraIndex]->GetViewMatrix(),
			totalTime);
		
		// Now that the shader has access to the correct world matrix, draw the entity's Mesh
		gameEntities[i]->Draw();
	}
}


// -----------------------------------------------------------------
// Sends colorTint and offset data to the constant buffer on the GPU
// -----------------------------------------------------------------
void Game::SendDataToConstantBuffer(XMFLOAT4 colorTint,
	XMFLOAT4X4 worldMatrix,
	XMFLOAT4X4 projectionMatrix,
	XMFLOAT4X4 viewMatrix,
	float totalTime)
{
	// Prepare vsData
	vsData.worldMatrix = worldMatrix;
	vsData.projectionMatrix = projectionMatrix;
	vsData.viewMatrix = viewMatrix;

	D3D11_MAPPED_SUBRESOURCE mappedVSBuffer = {};
	Graphics::Context->Map(vertexShaderConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedVSBuffer);

	memcpy(mappedVSBuffer.pData, &vsData, sizeof(vsData));

	Graphics::Context->Unmap(vertexShaderConstantBuffer.Get(), 0);

	Graphics::Context->VSSetConstantBuffers(
		0, // Which register to bind the buffer to? (b0)
		1, // How many are we setting right now?
		vertexShaderConstantBuffer.GetAddressOf()); // Array of buffers (or address of just one)

	// Prepare psData
	psData.colorTint = colorTint;
	psData.totalTime = totalTime;

	D3D11_MAPPED_SUBRESOURCE mappedPSBuffer = {};
	Graphics::Context->Map(pixelShaderConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedPSBuffer);
	
	memcpy(mappedPSBuffer.pData, &psData, sizeof(psData));
	
	Graphics::Context->Unmap(pixelShaderConstantBuffer.Get(), 0);
	
	Graphics::Context->PSSetConstantBuffers(
		0, // Which register to bind the buffer to? (b0)
		1, // How many are we setting right now?
	 	pixelShaderConstantBuffer.GetAddressOf()); // Array of buffers (or address of just one)
}


// ------------------------------
// Renders ImGui for Game::Draw()
// ------------------------------
void Game::RenderImGui()
{
	ImGui::Render(); // Turns this frame�s UI into renderable triangles
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