#include "Game.h"
#include "Graphics.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"
#include "Window.h"
// This code assumes files are in "ImGui" subfolder!
// Adjust as necessary for your own folder structure and project setup
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"
#include <DirectXMath.h>

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
	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	LoadShaders();
	CreateGeometry();

	// Initialize constant buffer data
	colorTint = XMFLOAT4(1.0f, 0.5f, 0.5f, 1.0f); // slight red tint
	offset = XMFLOAT3(0.1f, 0.0f, 0.0f);          // shift right


<<<<<<< Updated upstream
	// create the constant buffer
	D3D11_BUFFER_DESC cbDesc = {};
	cbDesc.ByteWidth = sizeof(VertexShaderExternalData);
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.MiscFlags = 0;
	cbDesc.StructureByteStride = 0;

	Graphics::Device->CreateBuffer(&cbDesc, nullptr, vsConstantBuffer.GetAddressOf());

	//bind constant buffer to vertex shader (slot 0)
	Graphics::Context->VSSetConstantBuffers(0, 1, vsConstantBuffer.GetAddressOf());

=======
>>>>>>> Stashed changes
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
		
		// initialize ImGui & platform/render backends
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui_ImplWin32_Init(Window::Handle());
		ImGui_ImplDX11_Init(Graphics::Device.Get(), Graphics::Context.Get());

		// Pick a style 
		ImGui::StyleColorsDark();
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
	// ImGui clean up
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
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
	XMFLOAT4 green = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 blue = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	XMFLOAT4 yellow = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);

	// Set up the vertices of the triangle we would like to draw
	// - We're going to copy this array, exactly as it exists in CPU memory
	//    over to a Direct3D-controlled data structure on the GPU (the vertex buffer)
	// - Note: Since we don't have a camera or really any concept of
	//    a "3d world" yet, we're simply describing positions within the
	//    bounds of how the rasterizer sees our screen: [-1 to +1] on X and Y
	// - This means (0,0) is at the very center of the screen.
	// - These are known as "Normalized Device Coordinates" or "Homogeneous 
	//    Screen Coords", which are ways to describe a position without
	//    knowing the exact size (in pixels) of the image/window/etc.  
	// - Long story short: Resizing the window also resizes the triangle,
	//    since we're describing the triangle in terms of the window itself
	Vertex triangleVertices[] =
	{
		{ XMFLOAT3(+0.0f, +0.5f, +0.0f), red }, 
		{ XMFLOAT3(+0.5f, -0.5f, +0.0f), blue },
		{ XMFLOAT3(-0.5f, -0.5f, +0.0f), green },
	};

	// Set up indices, which tell us which vertices to use and in which order
	// - This is redundant for just 3 vertices, but will be more useful later
	// - Indices are technically not required if the vertices are in the buffer 
	//    in the correct order and each one will be used exactly once
	// - But just to see how it's done...
	unsigned int triangleIndices[] = { 0, 1, 2 };

	// create the mesh using make_shared
	meshes.push_back(std::make_shared<Mesh>(
		triangleVertices,
		3, //vertex count
		triangleIndices,
		3, //index count
		Graphics::Device
	));

	//square
	Vertex squareVertices[] =
	{
		{ XMFLOAT3(-0.8f, +0.8f, +0.0f), red },		// top-left
		{ XMFLOAT3(-0.3f, +0.8f, +0.0f), blue },    // top-right
		{ XMFLOAT3(-0.3f, +0.3f, +0.0f), green },   // bottom-right
		{ XMFLOAT3(-0.8f, +0.3f, +0.0f), yellow }	// bottom-left
	};

	// indices for the square (two triangle)
	// triangle 1: top-left, top-right, bottom right (0,1,2)
	// triangle 2: top-left, bottom-right, bottom-left (0,2,3)
	unsigned int squareIndices[] = 
	{ 
		0, 1, 2,
		0, 2, 3 
	};

	meshes.push_back(std::make_shared<Mesh>(
		squareVertices,
		4,
		squareIndices,
		6, 
		Graphics::Device
	));

	//pentagon
	Vertex pentagonVertices[] =
	{
		// center vertex
		{ XMFLOAT3(+0.6f, +0.0f, +0.0f), red },

		// outer vertices
		{ XMFLOAT3(+0.6f, +0.4f, +0.0f), blue },      // top
		{ XMFLOAT3(+0.98f, +0.12f, +0.0f), green },   // top-right
		{ XMFLOAT3(+0.85f, -0.29f, +0.0f), yellow },  // bottom-right
		{ XMFLOAT3(+0.35f, -0.29f, +0.0f), green },   // bottom-left
		{ XMFLOAT3(+0.22f, +0.12f, +0.0f), blue }     // top-left
	};

	// indices for pentagon (5 triangles
	unsigned int pentagonIndices[] = {
		0, 1, 2,
		0, 2, 3,
		0, 3, 4,
		0, 4, 5,
		0, 5, 1
	};

	meshes.push_back(std::make_shared<Mesh>(
		pentagonVertices,
		6, 
		pentagonIndices,
		15, 
		Graphics::Device
	));
}

void BuildCustomWindow(float* color, bool* showDemoMenu, int* number, bool *showHappyMeter, DirectX::XMFLOAT4* colorTint, DirectX::XMFLOAT3* offset) {
	// create a new window
	ImGui::Begin("Custom Window");

	// display framerate %f is a replacable float, in sequence of inputs after
	ImGui::Text("fps: %f", ImGui::GetIO().Framerate);
	// display resolution %d is a decimal integer, in seqquence of inputs after
	ImGui::Text("resolution: %d x %d", Window::Width(), Window::Height());
	// edit the background color
	ImGui::ColorEdit4("background color", color);
	
	ImGui::Separator();
	ImGui::Text("Vertex Shader Controls");
	ImGui::ColorEdit4("Color Tint", &colorTint->x);
	ImGui::DragFloat3("Vertex Offset", &offset->x, 0.01f, -1.0f, 1.0f);
	
	//button to hide or show the window on click
	if (ImGui::Button("show demo menu")) {
		*showDemoMenu = !(*showDemoMenu);
	}
	if (ImGui::Button("surprise")) {
		*showHappyMeter = !(*showHappyMeter);
	}

	//end of building the window
	ImGui::End();
}
// --------------------------------------------------------
// Helper function to toggle visibility of happiness meter on new window
// --------------------------------------------------------
void NewWindowWithHappyMeter(int *number, std::string *happyMeterMessage) {

	// create a new window
	ImGui::Begin("Happiness 0-100??"); 

	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(13, 56, 46, 1));
	ImGui::PopStyleColor();

	if (*number == 100) {
		*happyMeterMessage = "   :]    ";
	}
	//create a slider for happiness level on this window
	ImGui::SliderInt((happyMeterMessage)->c_str(), number, 100, 100);
	ImGui::End();
}
void BuildMeshStatsWindow(const std::vector<std::shared_ptr<Mesh>>& meshes) {
	ImGui::Begin("Mesh Statistics");
	ImGui::Text("Total Meshes: %d", (int)meshes.size());
	ImGui::Separator();

	for (size_t i = 0; i < meshes.size(); i++) {
		const auto& mesh = meshes[i];
		int vertexCount = mesh->GetVertexCount();
		int indexCount = mesh->GetIndexCount();
		int triangleCount = indexCount / 3;

		ImGui::Text("Mesh %d", (int)i);
		ImGui::Indent();
		ImGui::Text("Vertices: %d", vertexCount);
		ImGui::Text("Indices: %d", indexCount);
		ImGui::Text("Triangles: %d", triangleCount);
		ImGui::Unindent();
		ImGui::Separator();
	}
	ImGui::End();
}

// --------------------------------------------------------
// Helper function to setup ImGui
// --------------------------------------------------------
void Game::ImGuiFresh(float deltaTime) {
	// Feed fresh data to ImGui
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = deltaTime;
	io.DisplaySize.x = (float)Window::Width();
	io.DisplaySize.y = (float)Window::Height();
	// Reset the frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	// Determine new input capture
	Input::SetKeyboardCapture(io.WantCaptureKeyboard);
	Input::SetMouseCapture(io.WantCaptureMouse);
	// Show the demo window
	//ImGui::ShowDemoWindow();

	// Show the demo window only if the boolean is true
	if (showDemoMenu)
	{
		ImGui::ShowDemoWindow();
	}

	//toggle the happiness meter
	if (showHappyMeter)
	{
		NewWindowWithHappyMeter(&number, &happyMeterMessage);
	}

	BuildCustomWindow(color, &showDemoMenu, &number, &showHappyMeter, &colorTint, &offset);
	BuildMeshStatsWindow(meshes);
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
	//call the function first to ensure it happens when a new frame has started
	ImGuiFresh(deltaTime);
	// Example input checking: Quit if the escape key is pressed
	if (Input::KeyDown(VK_ESCAPE))
		Window::Quit();
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
		//const float color[4] = { 0.4f, 0.6f, 0.75f, 0.0f };
		Graphics::Context->ClearRenderTargetView(Graphics::BackBufferRTV.Get(),	color);
		Graphics::Context->ClearDepthStencilView(Graphics::DepthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	//update constant buffer with current UI values
	{
		D3D11_MAPPED_SUBRESOURCE mappedBuffer = {};
		Graphics::Context->Map(vsConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);

		VertexShaderExternalData data;
		data.colorTint = colorTint;
		data.offset = offset;
		data.padding = 0.0f;

		memcpy(mappedBuffer.pData, &data, sizeof(VertexShaderExternalData));
		Graphics::Context->Unmap(vsConstantBuffer.Get(), 0);
	}

	// DRAW geometry
	// - These steps are generally repeated for EACH object you draw
	// - Other Direct3D calls will also be necessary to do more complex things
	{
		// Set buffers in the input assembler (IA) stage
		//  - Do this ONCE PER OBJECT, since each object may have different geometry
		//  - For this demo, this step *could* simply be done once during Init()
		//  - However, this needs to be done between EACH DrawIndexed() call
		//     when drawing different geometry, so it's here as an example
		//UINT stride = sizeof(Vertex);
		//UINT offset = 0;
		//Graphics::Context->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
		//Graphics::Context->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		//// Tell Direct3D to draw
		////  - Begins the rendering pipeline on the GPU
		////  - Do this ONCE PER OBJECT you intend to draw
		////  - This will use all currently set Direct3D resources (shaders, buffers, etc)
		////  - DrawIndexed() uses the currently set INDEX BUFFER to look up corresponding
		////     vertices in the currently set VERTEX BUFFER
		//Graphics::Context->DrawIndexed(
		//	3,     // The number of indices to use (we could draw a subset if we wanted)
		//	0,     // Offset to the first index we want to use
		//	0);    // Offset to add to each index when looking up vertices

		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		Graphics::Context->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
		
		for (auto& mesh : meshes) {
			mesh->Draw(Graphics::Context);
		}

		ImGui::Render(); // Turns this frame¡¦s UI into renderable triangles
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData()); // Draws it to the screen
	}

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



