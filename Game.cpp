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
	CreateShadowMapResources();
	CreatePostProcessResources();

	// Initialize constant buffer data
	colorTint = XMFLOAT4(1.0f, 0.5f, 0.5f, 1.0f); // slight red tint
	//offset = XMFLOAT3(0.1f, 0.0f, 0.0f);          // shift right
	ambientColor = XMFLOAT3(0.02f, 0.02f, 0.02f);
	//initialize the directional light
	//directionalLight1 = {};
	//directionalLight1.Type = LIGHT_TYPE_DIRECTIONAL;
	//directionalLight1.Direction = XMFLOAT3(1.0f, -1.0f, 0.0f); // points right and down
	//directionalLight1.Color = XMFLOAT3(1.0f, 1.0f, 1.0f);
	//directionalLight1.Intensity = 1.0f;

	//initialize the lights
	Light l1 = {};
	l1.Type = LIGHT_TYPE_DIRECTIONAL;
	l1.Direction = XMFLOAT3(1.0f, -1.0f, 0.0f);
	l1.Color = XMFLOAT3(1.0f, 1.0f, 1.0f); // white
	l1.Intensity = 1.0f;
	l1.CastsShadows = 1;
	lights.push_back(l1);

	//Light l2 = {};
	//l2.Type = LIGHT_TYPE_DIRECTIONAL;
	//l2.Direction = XMFLOAT3(-1.0f, -1.0f, 0.0f);
	//l2.Color = XMFLOAT3(0.0f, 1.0f, 0.0f); // green
	//l2.Intensity = 1.0f;
	//lights.push_back(l2);

	//Light l3 = {};
	//l3.Type = LIGHT_TYPE_SPOT;
	//l3.Position = XMFLOAT3(0.0f, 5.0f, 0.0f); // above the scene
	//l3.Direction = XMFLOAT3(0.0f, -1.0f, 0.0f); // pointing straight down
	//l3.Range = 15.0f;
	//l3.Color = XMFLOAT3(0.0f, 0.0f, 1.0f); // blue
	//l3.Intensity = 3.0f;
	//l3.SpotInnerAngle = 0.2f; // ~11 degrees
	//l3.SpotOuterAngle = 0.5f; // ~28 degrees
	//lights.push_back(l3);

	//Light l4 = {};
	//l4.Type = LIGHT_TYPE_POINT;
	//l4.Position = XMFLOAT3(0.0f, 2.0f, 0.0f); // hovering above center
	//l4.Range = 10.0f;
	//l4.Color = XMFLOAT3(1.0f, 1.0f, 0.0f); // yellow
	//l4.Intensity = 2.0f;
	//lights.push_back(l4);

	//Light l5 = {};
	//l5.Type = LIGHT_TYPE_POINT;
	//l5.Position = XMFLOAT3(-3.0f, 0.0f, 0.0f); // to the left
	//l5.Range = 8.0f;
	//l5.Color = XMFLOAT3(1.0f, 0.5f, 1.0f); // orange
	//l5.Intensity = 2.0f;
	//lights.push_back(l5);

	//// create the constant buffer
	//D3D11_BUFFER_DESC cbDesc = {};
	//cbDesc.ByteWidth = sizeof(VertexShaderExternalData);
	//cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	//cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	//cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	//cbDesc.MiscFlags = 0;
	//cbDesc.StructureByteStride = 0;
	//Graphics::Device->CreateBuffer(&cbDesc, nullptr, vsConstantBuffer.GetAddressOf());

	////bind constant buffer to vertex shader (slot 0)
	//Graphics::Context->VSSetConstantBuffers(0, 1, vsConstantBuffer.GetAddressOf());

	////create pixel shader constant buffer (for color tint)
	//D3D11_BUFFER_DESC psDesc = {};
	//psDesc.ByteWidth = sizeof(PixelShaderExternalData);
	//psDesc.Usage = D3D11_USAGE_DYNAMIC;
	//psDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	//psDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	//Graphics::Device->CreateBuffer(&psDesc, nullptr, psConstantBuffer.GetAddressOf());

	// Create large ring buffer constant buffer
	D3D11_BUFFER_DESC ringDesc = {};
	ringDesc.ByteWidth = cbRingBufferSize;
	ringDesc.Usage = D3D11_USAGE_DYNAMIC;
	ringDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	ringDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	Graphics::Device->CreateBuffer(&ringDesc, nullptr, cbRingBuffer.GetAddressOf());

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
		/*Graphics::Context->VSSetShader(vertexShader.Get(), 0, 0);
		Graphics::Context->PSSetShader(pixelShader.Get(), 0, 0);*/
		
		// initialize ImGui & platform/render backends
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui_ImplWin32_Init(Window::Handle());
		ImGui_ImplDX11_Init(Graphics::Device.Get(), Graphics::Context.Get());

		// Pick a style 
		ImGui::StyleColorsDark();
	}

	// -------------------------------------------------------
	// Create Cameras
	// -------------------------------------------------------
	float aspectRatio = (float)Window::Width() / Window::Height();

	// Camera 0: default position, normal FOV
	cameras.push_back(std::make_shared<Camera>(
		0.0f, 0.0f, -5.0f,		// position
		5.0f,					// move speed
		0.3f,					// look speed
		XM_PIDIV4,				// 45 degree FOV
		aspectRatio));

	// Camera 1: higher up, wider FOV
	cameras.push_back(std::make_shared<Camera>(
		0.0f, 3.0f, -8.0f,		// position
		8.0f,					// move speed (faster)
		0.2f,					// look speed
		XM_PI / 3.0f,			// 60 degree FOV
		aspectRatio));

	activeCameraIndex = 0;
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

// ===========================================================
// Create Post-Processing Resources
// ===========================================================
void Game::CreatePostProcessResources()
{
	// Clamp Sampler
	D3D11_SAMPLER_DESC sd = {};
	sd.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sd.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sd.MaxLOD = D3D11_FLOAT32_MAX;
	Graphics::Device->CreateSamplerState(&sd, ppSampler.GetAddressOf());

	// Helper Lambda
	auto LoadVS = [&](const wchar_t* path, ID3DBlob** blobOut)
		-> Microsoft::WRL::ComPtr<ID3D11VertexShader>
		{
			Microsoft::WRL::ComPtr<ID3D11VertexShader> shader;
			D3DReadFileToBlob(FixPath(path).c_str(), blobOut);
			if (*blobOut)
				Graphics::Device->CreateVertexShader(
					(*blobOut)->GetBufferPointer(), (*blobOut)->GetBufferSize(),
					nullptr, shader.GetAddressOf());
			return shader;
		};

	auto LoadPS = [&](const wchar_t* path)
		-> Microsoft::WRL::ComPtr<ID3D11PixelShader>
		{
			Microsoft::WRL::ComPtr<ID3D11PixelShader> shader;
			ID3DBlob* blob = nullptr;
			HRESULT hr = D3DReadFileToBlob(FixPath(path).c_str(), &blob);
			if (SUCCEEDED(hr) && blob)
			{
				Graphics::Device->CreatePixelShader(
					blob->GetBufferPointer(), blob->GetBufferSize(),
					nullptr, shader.GetAddressOf());
				blob->Release();
			}
			else
			{
				OutputDebugStringW(L"[PostProcess] FAILED to load shader: ");
				OutputDebugStringW(path);
				OutputDebugStringW(L"\n");
			}
			return shader;
		};

	// Load shaders
	ID3DBlob* vsBlob = nullptr;
	ppVS = LoadVS(L"PostProcessVS.cso", &vsBlob);
	if (vsBlob) vsBlob->Release();

	blurPS = LoadPS(L"BlurPS.cso");
	chromaPS = LoadPS(L"ChromaticAberrationPS.cso");

	// ── Create the two render targets at the current window size ─────────────
	auto MakeRT = [&](Microsoft::WRL::ComPtr<ID3D11RenderTargetView>& rtv,
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& srv,
		Microsoft::WRL::ComPtr<ID3D11Texture2D>& tex)
		{
			D3D11_TEXTURE2D_DESC td = {};
			td.Width = Window::Width();
			td.Height = Window::Height();
			td.MipLevels = 1;
			td.ArraySize = 1;
			td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			td.SampleDesc.Count = 1;
			td.Usage = D3D11_USAGE_DEFAULT;
			td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
			Graphics::Device->CreateTexture2D(&td, nullptr, tex.GetAddressOf());
			Graphics::Device->CreateRenderTargetView(tex.Get(), nullptr, rtv.GetAddressOf());
			Graphics::Device->CreateShaderResourceView(tex.Get(), nullptr, srv.GetAddressOf());
		};

	MakeRT(ppRTV, ppSRV, ppTexture);
	MakeRT(ppPingRTV, ppPingSRV, ppPingTexture);
}

// ============================================================
// Run Post Process Pass
// - This is a helper for running a full-screen post-process pass
// ============================================================
void Game::RunPostProcessPass(
	ID3D11PixelShader* ps,
	ID3D11ShaderResourceView* srcSRV,
	ID3D11RenderTargetView* dstRTV,
	void* cbData,
	size_t cbSize)
{
	// Bind destination, no depth
	Graphics::Context->OMSetRenderTargets(1, &dstRTV, nullptr);

	// Full-window viewport
	D3D11_VIEWPORT vp = {};
	vp.Width = (float)Window::Width();
	vp.Height = (float)Window::Height();
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	Graphics::Context->RSSetViewports(1, &vp);

	// Constant buffer (slot 0, pixel shader)
	FillAndBindNextConstantBuffer(cbData, cbSize, false, 0);

	// Shaders – no input layout for the VS_VertexID trick
	Graphics::Context->VSSetShader(ppVS.Get(), 0, 0);
	Graphics::Context->PSSetShader(ps, 0, 0);
	Graphics::Context->IASetInputLayout(nullptr);
	Graphics::Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Source texture + clamp sampler
	Graphics::Context->PSSetShaderResources(0, 1, &srcSRV);
	Graphics::Context->PSSetSamplers(0, 1, ppSampler.GetAddressOf());

	// Draw the full-screen triangle (no VB needed)
	Graphics::Context->Draw(3, 0);

	// Unbind source so D3D doesn't complain next frame
	ID3D11ShaderResourceView* nullSRV = nullptr;
	Graphics::Context->PSSetShaderResources(0, 1, &nullSRV);
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
	ID3DBlob* shadowBlob;

	Microsoft::WRL::ComPtr<ID3D11VertexShader> vs;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> ps;
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
		D3DReadFileToBlob(FixPath(L"ShadowVS.cso").c_str(), &shadowBlob);

		// Create the actual Direct3D shaders on the GPU
		Graphics::Device->CreatePixelShader(
			pixelShaderBlob->GetBufferPointer(),	// Pointer to blob's contents
			pixelShaderBlob->GetBufferSize(),		// How big is that data?
			0,										// No classes in this shader
			ps.GetAddressOf());			// Address of the ID3D11PixelShader pointer

		Graphics::Device->CreateVertexShader(
			vertexShaderBlob->GetBufferPointer(),	// Get a pointer to the blob's contents
			vertexShaderBlob->GetBufferSize(),		// How big is that data?
			0,										// No classes in this shader
			vs.GetAddressOf());			// The address of the ID3D11VertexShader pointer

		Graphics::Device->CreateVertexShader(
			shadowBlob->GetBufferPointer(),
			shadowBlob->GetBufferSize(),
			nullptr,
			shadowVertexShader.GetAddressOf());

	}

	// Create an input layout 
	//  - This describes the layout of data sent to a vertex shader
	//  - In other words, it describes how to interpret data (numbers) in a vertex buffer
	//  - Doing this NOW because it requires a vertex shader's byte code to verify against!
	//  - Luckily, we already have that loaded (the vertex shader blob above)
	{
		D3D11_INPUT_ELEMENT_DESC inputElements[4] = {};

		// Set up the first element - a position, which is 3 float values
		inputElements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;				// Most formats are described as color channels; really it just means "Three 32-bit floats"
		inputElements[0].SemanticName = "POSITION";							// This is "POSITION" - needs to match the semantics in our vertex shader input!
		inputElements[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// How far into the vertex is this?  Assume it's after the previous element

		// Set up the second element - a normal, which is 4 more float values
		inputElements[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;			// 4x 32-bit floats
		inputElements[1].SemanticName = "NORMAL";							// Match our vertex shader input!
		inputElements[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// After the previous element

		inputElements[2].SemanticName = "TEXCOORD";
		inputElements[2].Format = DXGI_FORMAT_R32G32_FLOAT;
		inputElements[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;

		inputElements[3].SemanticName = "TANGENT";
		inputElements[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		inputElements[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;

		// Create the input layout, verifying our description against actual shader code
		Graphics::Device->CreateInputLayout(
			inputElements,							// An array of descriptions
			4,										// How many elements in that array?
			vertexShaderBlob->GetBufferPointer(),	// Pointer to the code of a shader that uses this layout
			vertexShaderBlob->GetBufferSize(),		// Size of the shader code that uses this layout
			inputLayout.GetAddressOf());			// Address of the resulting ID3D11InputLayout pointer

		vertexShaderBlob->Release();
		pixelShaderBlob->Release();
		shadowBlob->Release();

		// Helper lambda to load a pixel shader from a .cso file
		auto LoadPS = [&](const wchar_t* path) -> Microsoft::WRL::ComPtr<ID3D11PixelShader>
			{
				Microsoft::WRL::ComPtr<ID3D11PixelShader> ps;
				ID3DBlob* blob = nullptr;

				HRESULT hr = D3DReadFileToBlob(FixPath(path).c_str(), &blob);
				if (FAILED(hr) || blob == nullptr)
				{
					// Will tell you exactly which file is missing
					OutputDebugStringW(L"FAILED to load shader: ");
					OutputDebugStringW(path);
					OutputDebugStringW(L"\n");
					return nullptr;  // return null so you get a cleaner error
				}

				Graphics::Device->CreatePixelShader(
					blob->GetBufferPointer(), blob->GetBufferSize(), 0, ps.GetAddressOf());
				blob->Release();
				return ps;
			};

		// --- Load each pixel shader ---
		auto psTextured = LoadPS(L"PixelShader.cso");
		auto psMultiTexture = LoadPS(L"CustomTexture.cso");
		auto psColorTint = LoadPS(L"PSColorTint.cso");
		auto psUV = LoadPS(L"PSUVData.cso");
		auto psNormal = LoadPS(L"PSNormalData.cso");
		auto psCustom = LoadPS(L"PSCustom.cso");

		// Load textures
		HRESULT hr1 = DirectX::CreateWICTextureFromFile(
			Graphics::Device.Get(), Graphics::Context.Get(),
			FixPath(L"../../Assets/Textures/PBR/cobblestone_albedo.png").c_str(),
			nullptr, srvCobblestoneTexture.GetAddressOf());

		HRESULT hr2 = DirectX::CreateWICTextureFromFile(
			Graphics::Device.Get(), Graphics::Context.Get(),
			FixPath(L"../../Assets/Textures/PBR/bronze_albedo.png").c_str(),
			nullptr, srvBronzeTexture.GetAddressOf());

		HRESULT hr3 = DirectX::CreateWICTextureFromFile(
			Graphics::Device.Get(), Graphics::Context.Get(),
			FixPath(L"../../Assets/Textures/PBR/wood_albedo.png").c_str(),
			nullptr, srvFloorTexture.GetAddressOf());


		//Load normal maps
		HRESULT hr4 = DirectX::CreateWICTextureFromFile(
				Graphics::Device.Get(), Graphics::Context.Get(),
				FixPath(L"../../Assets/Textures/PBR/cobblestone_normals.png").c_str(),
			nullptr, srvCobblestoneNormalMap.GetAddressOf());

		HRESULT hr5 = DirectX::CreateWICTextureFromFile(
			Graphics::Device.Get(), Graphics::Context.Get(),
			FixPath(L"../../Assets/Textures/PBR/bronze_normals.png").c_str(),
			nullptr, srvBronzeNormalMap.GetAddressOf());

		HRESULT hr6 = DirectX::CreateWICTextureFromFile(
			Graphics::Device.Get(), Graphics::Context.Get(),
			FixPath(L"../../Assets/Textures/PBR/wood_normals.png").c_str(),
			nullptr, srvFloorNormalMap.GetAddressOf());

		HRESULT hr7 = DirectX::CreateWICTextureFromFile(
			Graphics::Device.Get(), Graphics::Context.Get(),
			FixPath(L"../../Assets/Textures/Textures with Normal Maps/Textures with Normal Maps/flat_normals.png").c_str(),
			nullptr, srvFlatNormalMap.GetAddressOf());


		if (FAILED(hr1)) OutputDebugStringW(L"Texture 1 FAILED to load\n");
		if (FAILED(hr2)) OutputDebugStringW(L"Texture 2 FAILED to load\n");
		if (FAILED(hr3)) OutputDebugStringW(L"Normal map FAILED to load\n");
		if (FAILED(hr4)) OutputDebugStringW(L"Damaged plaster normal map FAILED to load\n");
		if (FAILED(hr5)) OutputDebugStringW(L"Flat normal map FAILED to load\n");

		//load roughness and metalness maps
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvCobblestoneRoughness;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvCobblestoneMetalness;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvBronzeRoughness;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvBronzeMetalness;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvFloorRoughness;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvFloorMetalness;

		CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(),
			FixPath(L"../../Assets/Textures/PBR/cobblestone_roughness.png").c_str(),
			nullptr, srvCobblestoneRoughness.GetAddressOf());

		CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(),
			FixPath(L"../../Assets/Textures/PBR/cobblestone_metal.png").c_str(),
			nullptr, srvCobblestoneMetalness.GetAddressOf());

		CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(),
			FixPath(L"../../Assets/Textures/PBR/bronze_roughness.png").c_str(),
			nullptr, srvBronzeRoughness.GetAddressOf());

		CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(),
			FixPath(L"../../Assets/Textures/PBR/bronze_metal.png").c_str(),
			nullptr, srvBronzeMetalness.GetAddressOf());

		CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(),
			FixPath(L"../../Assets/Textures/PBR/wood_roughness.png").c_str(),
			nullptr, srvFloorRoughness.GetAddressOf());

		CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(),
			FixPath(L"../../Assets/Textures/PBR/wood_metal.png").c_str(),
			nullptr, srvFloorMetalness.GetAddressOf());

		// Create sampler state
		D3D11_SAMPLER_DESC sampDesc = {};
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
		sampDesc.MaxAnisotropy = 16;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
		Graphics::Device->CreateSamplerState(&sampDesc, samplerState.GetAddressOf());

		// --- Materials ---
		// materials[0]: color tint, textured
		materials.push_back(std::make_shared<Material>(XMFLOAT4(1, 1, 1, 1), vs, psTextured));
		// materials[1]: color tint, textured
		materials.push_back(std::make_shared<Material>(XMFLOAT4(1, 0, 0, 1), vs, psTextured));
		// materials[2]: color tint, textured
		materials.push_back(std::make_shared<Material>(XMFLOAT4(1, 1, 1, 1), vs, psTextured));
		// materials[3]: UV debug
		materials.push_back(std::make_shared<Material>(XMFLOAT4(1, 1, 1, 1), vs, psUV));
		// materials[4]: Normal debug
		materials.push_back(std::make_shared<Material>(XMFLOAT4(1, 1, 1, 1), vs, psNormal));
		// materials[5]: Custom
		materials.push_back(std::make_shared<Material>(XMFLOAT4(1, 1, 1, 1), vs, psCustom));
		// materials[6]: Custom Texture
		materials.push_back(std::make_shared<Material>(XMFLOAT4(1, 1, 1, 1), vs, psMultiTexture));


		// Assign texture1 + sampler to materials 0 and 1
		materials[0]->AddTextureSRV(0, srvCobblestoneTexture);
		materials[0]->AddTextureSRV(1, srvCobblestoneNormalMap);
		materials[0]->AddTextureSRV(2, srvCobblestoneRoughness);
		materials[0]->AddTextureSRV(3, srvCobblestoneMetalness);
		materials[0]->AddSampler(0, samplerState);

		materials[1]->AddTextureSRV(0, srvBronzeTexture);
		materials[1]->AddTextureSRV(1, srvBronzeNormalMap);
		materials[1]->AddTextureSRV(2, srvBronzeRoughness);
		materials[1]->AddTextureSRV(3, srvBronzeMetalness);
		materials[1]->AddSampler(0, samplerState);

		// Assign texture2 to material 2 (also uses psTextured)
		materials[2]->AddTextureSRV(0, srvFloorTexture);
		materials[2]->AddTextureSRV(1, srvFloorNormalMap);
		materials[2]->AddTextureSRV(2, srvFloorRoughness);
		materials[2]->AddTextureSRV(3, srvFloorMetalness);
		materials[2]->AddSampler(0, samplerState);

		// Materials 3-5 use debug/custom shaders, texture assignment optional
		for (int i = 3; i <= 5; i++) {
			materials[i]->AddTextureSRV(1, srvFloorNormalMap);
			materials[i]->AddSampler(0, samplerState);
		}
		// assign both textures to the 6th material, multi-texture and sampler state
		materials[6]->AddTextureSRV(0, srvFloorTexture);
		materials[6]->AddTextureSRV(1, srvBronzeTexture);
		materials[6]->AddSampler(0, samplerState);
	}
}


// --------------------------------------------------------
// Creates the geometry we're going to draw
// --------------------------------------------------------
void Game::CreateGeometry()
{
	//// Create some temporary variables to represent colors
	//// - Not necessary, just makes things more readable
	//XMFLOAT4 red = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	//XMFLOAT4 green = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	//XMFLOAT4 blue = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	//XMFLOAT4 yellow = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);

	//// Set up the vertices of the triangle we would like to draw
	//// - We're going to copy this array, exactly as it exists in CPU memory
	////    over to a Direct3D-controlled data structure on the GPU (the vertex buffer)
	//// - Note: Since we don't have a camera or really any concept of
	////    a "3d world" yet, we're simply describing positions within the
	////    bounds of how the rasterizer sees our screen: [-1 to +1] on X and Y
	//// - This means (0,0) is at the very center of the screen.
	//// - These are known as "Normalized Device Coordinates" or "Homogeneous 
	////    Screen Coords", which are ways to describe a position without
	////    knowing the exact size (in pixels) of the image/window/etc.  
	//// - Long story short: Resizing the window also resizes the triangle,
	////    since we're describing the triangle in terms of the window itself
	//Vertex triangleVertices[] =
	//{
	//	{ XMFLOAT3(+0.0f, +0.5f, +0.0f), red }, 
	//	{ XMFLOAT3(+0.5f, -0.5f, +0.0f), blue },
	//	{ XMFLOAT3(-0.5f, -0.5f, +0.0f), green },
	//};

	//// Set up indices, which tell us which vertices to use and in which order
	//// - This is redundant for just 3 vertices, but will be more useful later
	//// - Indices are technically not required if the vertices are in the buffer 
	////    in the correct order and each one will be used exactly once
	//// - But just to see how it's done...
	//unsigned int triangleIndices[] = { 0, 1, 2 };

	//// create the mesh using make_shared
	//meshes.push_back(std::make_shared<Mesh>(
	//	triangleVertices,
	//	3, //vertex count
	//	triangleIndices,
	//	3, //index count
	//	Graphics::Device
	//));

	////square
	//Vertex squareVertices[] =
	//{
	//	{ XMFLOAT3(-0.8f, +0.8f, +0.0f), red },		// top-left
	//	{ XMFLOAT3(-0.3f, +0.8f, +0.0f), blue },    // top-right
	//	{ XMFLOAT3(-0.3f, +0.3f, +0.0f), green },   // bottom-right
	//	{ XMFLOAT3(-0.8f, +0.3f, +0.0f), yellow }	// bottom-left
	//};

	//// indices for the square (two triangle)
	//// triangle 1: top-left, top-right, bottom right (0,1,2)
	//// triangle 2: top-left, bottom-right, bottom-left (0,2,3)
	//unsigned int squareIndices[] = 
	//{ 
	//	0, 1, 2,
	//	0, 2, 3 
	//};

	//meshes.push_back(std::make_shared<Mesh>(
	//	squareVertices,
	//	4,
	//	squareIndices,
	//	6, 
	//	Graphics::Device
	//));

	////pentagon
	//Vertex pentagonVertices[] =
	//{
	//	// center vertex
	//	{ XMFLOAT3(+0.6f, +0.0f, +0.0f), red },

	//	// outer vertices
	//	{ XMFLOAT3(+0.6f, +0.4f, +0.0f), blue },      // top
	//	{ XMFLOAT3(+0.98f, +0.12f, +0.0f), green },   // top-right
	//	{ XMFLOAT3(+0.85f, -0.29f, +0.0f), yellow },  // bottom-right
	//	{ XMFLOAT3(+0.35f, -0.29f, +0.0f), green },   // bottom-left
	//	{ XMFLOAT3(+0.22f, +0.12f, +0.0f), blue }     // top-left
	//};

	//// indices for pentagon (5 triangles
	//unsigned int pentagonIndices[] = {
	//	0, 1, 2,
	//	0, 2, 3,
	//	0, 3, 4,
	//	0, 4, 5,
	//	0, 5, 1
	//};

	//meshes.push_back(std::make_shared<Mesh>(
	//	pentagonVertices,
	//	6, 
	//	pentagonIndices,
	//	15, 
	//	Graphics::Device
	//));

	// load meshes from the OBJ file
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/cube.obj").c_str(), Graphics::Device));
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/sphere.obj").c_str(), Graphics::Device));
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/helix.obj").c_str(), Graphics::Device));

	// create All entitities
	//-----------------------------------------------------------------	
	// entity 0: cube, left, will rotate
	entities.push_back(GameEntity(meshes[0], materials[0]));
	entities[0].GetTransform()->SetPosition(-2.0f, 0.0f, 0.0f);

	// entity 1: sphere, center, will move in sine wave
	entities.push_back(GameEntity(meshes[1], materials[1]));
	entities[1].GetTransform()->SetPosition(0.0f, 0.0f, 0.0f);

	// entity 2: helix, right, will scale up and down
	//entities.push_back(GameEntity(meshes[2], materials[2]));
	//entities[2].GetTransform()->SetPosition(4.0f, 0.0f, 0.0f);

	//entity 3: cube, center, will move left and right
	//entities.push_back(GameEntity(meshes[0], materials[1]));
	//entities[2].GetTransform()->SetPosition(-4.5f, 4.0f, 0.0f);

	// entity 4: sphere (shared mesh), will rotate opposite direction
	entities.push_back(GameEntity(meshes[1], materials[2]));
	entities[2].GetTransform()->SetPosition(2.0f, 2.0f, 4.0f);

	// entity 5: cube, colorTint - red
	entities.push_back(GameEntity(meshes[0], materials[0]));
	entities[3].GetTransform()->SetPosition(0.0f, 4.0f, 0.0f);

	// entity 6: helix, colorTint - blue
	entities.push_back(GameEntity(meshes[2], materials[1]));
	entities[4].GetTransform()->SetPosition(2.0f, 4.0f, 0.0f);

	//entity 7:cube, uv
	entities.push_back(GameEntity(meshes[0], materials[3]));
	entities[5].GetTransform()->SetPosition(-2.0f, 4.0f, 0.0f);
	// entity 8: helix, uv
	entities.push_back(GameEntity(meshes[2], materials[3]));
	entities[6].GetTransform()->SetPosition(-4.0f, 4.0f, 0.0f);

	//entity 9:cube, normal
	//entities.push_back(GameEntity(meshes[1], materials[4]));
	//entities[7].GetTransform()->SetPosition(-2.0f, 6.0f, 0.0f);
	// entity 10: helix, normal
	//entities.push_back(GameEntity(meshes[2], materials[4]));
	//entities[8].GetTransform()->SetPosition(-4.0f, 6.0f, 0.0f);

	//entity 11:cube, custom
	entities.push_back(GameEntity(meshes[0], materials[5]));
	entities[7].GetTransform()->SetPosition(0.0f, 6.0f, 0.0f);
	// entity 12: helix, custom
	entities.push_back(GameEntity(meshes[2], materials[5]));
	entities[8].GetTransform()->SetPosition(2.0f, 6.0f, 0.0f);

	// entity 13: sphere, custom texture
	entities.push_back(GameEntity(meshes[1], materials[6])); 
	entities[9].GetTransform()->SetPosition(-6.0f, 0.0f, 0.0f);

	// create the floor
	entities.push_back(GameEntity(meshes[0], materials[2])); // use light-colored material
	entities[10].GetTransform()->SetPosition(0.0f, -3.0f, 0.0f); // below everything
	entities[10].GetTransform()->SetScale(20.0f, 0.5f, 20.0f);   // wide and flat

	// Create the sky
	sky = std::make_shared<Sky>(
		meshes[0],
		samplerState,
		L"../../Assets/Textures/Skies/Skies/Planet/right.png",
		L"../../Assets/Textures/Skies/Skies/Planet/left.png",
		L"../../Assets/Textures/Skies/Skies/Planet/up.png",
		L"../../Assets/Textures/Skies/Skies/Planet/down.png",
		L"../../Assets/Textures/Skies/Skies/Planet/front.png",
		L"../../Assets/Textures/Skies/Skies/Planet/back.png",
		L"SkyVS.cso",
		L"SkyPS.cso"
	);
}

void Game::FillAndBindNextConstantBuffer(void* data, size_t dataSize,
	bool isVertexShader, UINT slot) 
{
	size_t alignedSize = (dataSize + 255) & ~255;

	if (cbRingBufferOffset + alignedSize > cbRingBufferSize)
		cbRingBufferOffset = 0;

	D3D11_MAPPED_SUBRESOURCE mapped = {};
	Graphics::Context->Map(cbRingBuffer.Get(), 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &mapped);
	memcpy((char*)mapped.pData + cbRingBufferOffset, data, dataSize);
	Graphics::Context->Unmap(cbRingBuffer.Get(), 0);

	UINT firstConstant = (UINT)(cbRingBufferOffset / 16);
	UINT numConstants = (UINT)(alignedSize / 16);

	if (isVertexShader)
		Graphics::Context->VSSetConstantBuffers1(slot, 1, cbRingBuffer.GetAddressOf(),
			&firstConstant, &numConstants);
	else
		Graphics::Context->PSSetConstantBuffers1(slot, 1, cbRingBuffer.GetAddressOf(),
			&firstConstant, &numConstants);

	cbRingBufferOffset += alignedSize;
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
	// change the background color of this window to something different
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(13/255.0f, 56 / 255.0f, 46 / 255.0f, 1));
	// create a new window
	ImGui::Begin("Happiness 0-100??");
	//UI stuff
	if (*number == 100) {
		*happyMeterMessage = "   :]    ";
	}
	//create a slider for happiness level on this window
	ImGui::SliderInt((happyMeterMessage)->c_str(), number, 100, 100);
	ImGui::End();
	ImGui::PopStyleColor();
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

	// Main Window
	// ---------------------------------------------------------------------
	ImGui::Begin("Game Info");
	ImGui::Text("fps: %.1f", ImGui::GetIO().Framerate); 
	ImGui::Text("resolution: %d x %d", Window::Width(), Window::Height());
	ImGui::ColorEdit4("Background", color); 
	ImGui::Separator();
	ImGui::ColorEdit4("Color Tint", &colorTint.x);
	if (ImGui::Button("Toggle Demo")) 
		showDemoMenu = !showDemoMenu; 
	ImGui::End(); 

	// Camera window
	// ---------------------------------------------------------------------
	ImGui::Begin("Cameras");

	//Camera switched - one button per camera
	for(int i = 0; i < (int)cameras.size(); i++)
	{
		char label[32];
		sprintf_s(label, "Camera %d", i);

		//highlight the active camera's button
		if (i == activeCameraIndex)
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f)); // green for active
		
		if (ImGui::Button(label))
			activeCameraIndex = i; 

		if (i == activeCameraIndex)
			ImGui::PopStyleColor();

		ImGui::SameLine();
	}
	ImGui::NewLine();
	ImGui::Separator();

	// active camera details
	auto& cam = cameras[activeCameraIndex];
	Transform& t = cam->GetTransform();

	XMFLOAT3 pos = t.GetPosition();
	XMFLOAT3 rot = t.GetPitchYawRoll();
	XMFLOAT3 fwd = t.GetForward();

	ImGui::Text("Active: Camera %d", activeCameraIndex);
	ImGui::Text("Position:  %.2f, %.2f, %.2f", pos.x, pos.y, pos.z);
	ImGui::Text("Rotation:  %.2f, %.2f, %.2f", rot.x, rot.y, rot.z);
	ImGui::Text("Forward:   %.2f, %.2f, %.2f", fwd.x, fwd.y, fwd.z);
	ImGui::Text("FOV:       %.1f degrees", XMConvertToDegrees(cam->GetFOV()));
	ImGui::Text("MoveSpeed: %.1f", cam->GetMoveSpeed());
	ImGui::Text("LookSpeed: %.1f", cam->GetLookSpeed());

	ImGui::End();

	// entity inspector
	// -----------------------------------------------------------------------
	ImGui::Begin("Entities");
	for (int i = 0; i < (int)entities.size(); i++)
	{
		// push a unique ID per entity so duplicate labels don't conflict
		ImGui::PushID(i);

		char label[32];
		sprintf_s(label, "Entity %d", i);

		if (ImGui::CollapsingHeader(label))
		{
			Transform* t = entities[i].GetTransform();

			XMFLOAT3 pos = t->GetPosition();
			XMFLOAT3 rot = t->GetPitchYawRoll();
			XMFLOAT3 scl = t->GetScale();

			if (ImGui::DragFloat3("Position", &pos.x, 0.01f))
				t->SetPosition(pos);
			if (ImGui::DragFloat3("Rotation", &rot.x, 0.01f))
				t->SetRotation(rot);
			if (ImGui::DragFloat3("Scale", &scl.x, 0.01f, 0.01f, 10.0f))
				t->SetScale(scl);

			ImGui::Text("Mesh indices: %d", entities[i].GetMesh()->GetIndexCount());

			// Material Details
			ImGui::Separator();
			ImGui::Text("Material");

			auto mat = entities[i].GetMaterial();

			// Color Tint
			XMFLOAT4 tint = mat->GetColorTint();
			if (ImGui::ColorEdit4("ColorTint", &tint.x))
				mat->SetColorTint(tint);

			//UV scale 
			XMFLOAT2 uvScale = mat->GetUVScale();
			if (ImGui::DragFloat2("UV Scale", &uvScale.x, 0.1f, 0.0f, 10.0f))
				mat->SetUVScale(uvScale);

			//UV offset
			XMFLOAT2 uvOffset = mat->GetUVOffset();
			if (ImGui::DragFloat2("UV Offset", &uvOffset.x, 0.01f, -5.0f, 5.0f))
				mat->SetUVOffset(uvOffset);
		}
		ImGui::PopID();
	}
	ImGui::End();

	//Lights Window
	ImGui::Begin("Lights");

	//Ambient color control
	ImGui::ColorEdit3("Ambient Color", &ambientColor.x);
	ImGui::Separator();

	// per-light controls
	for (int i = 0; i < (int)lights.size(); i++) {
		ImGui::PushID(i);

		char label[32];
		sprintf_s(label, "Light %d", i);

		if (ImGui::CollapsingHeader(label)) {
			//color
			ImGui::ColorEdit3("Color", &lights[i].Color.x);
			//intensity
			ImGui::DragFloat("Intensity", &lights[i].Intensity, 0.01f, 0.0f, 10.0f);
			//direction (for directional and spotlights)
			if (lights[i].Type == LIGHT_TYPE_DIRECTIONAL || lights[i].Type == LIGHT_TYPE_SPOT) {
				if (ImGui::DragFloat3("Direction", &lights[i].Direction.x, 0.01f, -1.0f, 1.0f)) {
					//normalize after use changes it
					XMVECTOR dir = XMLoadFloat3(&lights[i].Direction);
					dir = XMVector3Normalize(dir);
					XMStoreFloat3(&lights[i].Direction, dir);
				}

			}

			// position (for point and spotlights)
			if (lights[i].Type == LIGHT_TYPE_POINT || lights[i].Type == LIGHT_TYPE_SPOT) {
				ImGui::DragFloat3("Position", &lights[i].Position.x, 0.1f);
				ImGui::DragFloat3("Range", &lights[i].Range, 0.1f, 0.0f, 100.0f);
			}
		}
		ImGui::PopID();
	}
	ImGui::End();

	// Post-Process controls
	ImGui::Begin("Post Process");

	ImGui::SeparatorText("Blur (Task 1)");
	ImGui::Checkbox("Enable Blur", &blurEnabled);
	if (blurEnabled)
	{
		ImGui::SliderInt("Blur Radius", &blurRadius, 0, 10);
		ImGui::TextDisabled("0 = no blur  |  cost scales with radius^2");
	}

	ImGui::SeparatorText("Chromatic Aberration (Task 2)");
	ImGui::Checkbox("Enable Chromatic Aberration", &chromaEnabled);
	if (chromaEnabled)
	{
		ImGui::SliderFloat("Strength", &chromaStrength, 0.0f, 0.03f, "%.4f");
		ImGui::TextDisabled("0 = no effect  |  stronger at screen edges");
	}

	ImGui::End();
	// -------------------------------------------------------------------

	// mesh stats window
	BuildMeshStatsWindow(meshes);
}

// --------------------------------------------------------
// Handle resizing to match the new window size
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
	float aspectRatio = (float)Window::Width() / Window::Height();
	for(auto& cam : cameras)
		cam->UpdateProjectionMatrix(aspectRatio);
}

void Game::CreateShadowMapResources() {
	const int SHADOW_MAP_SIZE = 1024;

	// create the shadow map texture
	Microsoft::WRL::ComPtr<ID3D11Texture2D> shadowTexture;
	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = SHADOW_MAP_SIZE;
	texDesc.Height = SHADOW_MAP_SIZE;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R32_TYPELESS; // typeless, can create both SRV and DSV
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	Graphics::Device->CreateTexture2D(&texDesc, nullptr, shadowTexture.GetAddressOf());

	//DSV - depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	Graphics::Device->CreateDepthStencilView(shadowTexture.Get(), &dsvDesc, shadowDSV.GetAddressOf());

	//SRV - shader resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	Graphics::Device->CreateShaderResourceView(shadowTexture.Get(), &srvDesc, shadowSRV.GetAddressOf());

	//Rasterizer stater 
	D3D11_RASTERIZER_DESC rasterDesc = {};
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.DepthClipEnable = false;
	rasterDesc.DepthBias = 1000;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.SlopeScaledDepthBias = 1.0f;
	Graphics::Device->CreateRasterizerState(&rasterDesc, shadowRasterizer.GetAddressOf());

	XMFLOAT3 lightDir = XMFLOAT3(1.0f, -1.0f, 0.0f);
	XMVECTOR dir = XMVector3Normalize(XMLoadFloat3(&lightDir));

	XMVECTOR lightPos = XMVectorScale(dir, -20.0f);

	XMMATRIX view = XMMatrixLookAtLH(
		lightPos,
		XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
		XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
	XMStoreFloat4x4(&lightViewMatrix, view);

	//comparison sampler state for shadow map sampling
	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	sampDesc.BorderColor[0] = 1.0f;
	sampDesc.BorderColor[1] = 1.0f;
	sampDesc.BorderColor[2] = 1.0f;
	sampDesc.BorderColor[3] = 1.0f;
	Graphics::Device->CreateSamplerState(&sampDesc, shadowSampler.GetAddressOf());

	//orthographic projection for directional light
	XMMATRIX proj = XMMatrixOrthographicLH(20.0f, 20.0f, 0.1f, 100.0f);
	XMStoreFloat4x4(&lightProjectionMatrix, proj);
}

void Game::RenderShadowMap() {
	// clear the shadow map depth
	Graphics::Context->ClearDepthStencilView(shadowDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	Graphics::Context->OMSetRenderTargets(0, 0, shadowDSV.Get());
	//Apply shadow rasterizer
	Graphics::Context->RSSetState(shadowRasterizer.Get());

	// Set shadow map viewport to match texture size
	D3D11_VIEWPORT vp = {};
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.Width = 1024.0f;
	vp.Height = 1024.0f;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	Graphics::Context->RSSetViewports(1, &vp);


	// shadow VS only, no pixel shader
	Graphics::Context->VSSetShader(shadowVertexShader.Get(), 0, 0);
	Graphics::Context->PSSetShader(0, 0, 0);

	// Simple struct - just world/view/proj
	struct ShadowVSData
	{
		XMFLOAT4X4 world;
		XMFLOAT4X4 view;
		XMFLOAT4X4 proj;
	};

	ShadowVSData vsData = {};
	vsData.view = lightViewMatrix;
	vsData.proj = lightProjectionMatrix;

	for (auto& entity : entities) {
		vsData.world = entity.GetTransform()->GetWorldMatrix();
		// Use whatever cbuffer binding you have
		FillAndBindNextConstantBuffer(&vsData, sizeof(ShadowVSData), true, 0);
		entity.Draw(Graphics::Context);
	}

	// restore everything
	Graphics::Context->OMSetRenderTargets(1, Graphics::BackBufferRTV.GetAddressOf(), Graphics::DepthBufferDSV.Get());
	vp.Width = (float)Window::Width();
	vp.Height = (float)Window::Height();
	Graphics::Context->RSSetViewports(1, &vp);
	Graphics::Context->RSSetState(0);
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

	// update only active camera
	cameras[activeCameraIndex]->Update(deltaTime);

	// entity 0: rotate continuously
	entities[0].GetTransform()->Rotate(0.0f, 0.0f, 1.0f * deltaTime);
	// entity 1: move in a horizontal sine wave
	{
		float s = sinf(totalTime * 2.0f) * 0.005f;
		entities[1].GetTransform()->SetPosition(s, 0.0f, 0.0f);
	}
	// entity 2: pulsate scale
	{
		float pulse = 1.0f + 0.3f * sinf(totalTime * 3.0f);
		entities[2].GetTransform()->SetScale(pulse, pulse, 1.0f);
	}
	// entity 3: move left and right
	{
		float x = sinf(totalTime) * 0.4f;
		entities[3].GetTransform()->SetPosition(x, -0.5f, 0.0f);
	}
	// entity 4: rotate opposite direction
	entities[4].GetTransform()->Rotate(0.0f, 0.0f, -0.8f * deltaTime);
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
		Graphics::Context->ClearRenderTargetView(ppRTV.Get(), color);
		Graphics::Context->ClearDepthStencilView(Graphics::DepthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

		// restore the input layout for scene geometry
		Graphics::Context->IASetInputLayout(inputLayout.Get());
		Graphics::Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// redirect scene rendering to the post-process render target
		Graphics::Context->OMSetRenderTargets(1, ppRTV.GetAddressOf(), Graphics::DepthBufferDSV.Get());
	}

	//A4
	//update constant buffer with current UI values
	//{
	//	D3D11_MAPPED_SUBRESOURCE mappedBuffer = {};
	//	Graphics::Context->Map(vsConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);

	//	VertexShaderExternalData data;
	//	data.colorTint = colorTint;
	//	//data.offset = offset;
	//	data.padding = 0.0f;

	//	memcpy(mappedBuffer.pData, &data, sizeof(VertexShaderExternalData));
	//	Graphics::Context->Unmap(vsConstantBuffer.Get(), 0);
	//}

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

		/*UINT stride = sizeof(Vertex);
		UINT offset = 0;
		Graphics::Context->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
		
		for (auto& mesh : meshes) {
			mesh->Draw(Graphics::Context);
		}*/

		//a6
		// grab active camera matrices
		XMFLOAT4X4 view = cameras[activeCameraIndex]->GetViewMatrix();
		XMFLOAT4X4 projection = cameras[activeCameraIndex]->GetProjectionMatrix();

		RenderShadowMap();

		Graphics::Context->OMSetRenderTargets(1, ppRTV.GetAddressOf(), Graphics::DepthBufferDSV.Get());

		//bind shadowSRV and shadowSampler to the pixel shader for use in lighting calculations
		Graphics::Context->PSSetShaderResources(4, 1, shadowSRV.GetAddressOf());
		Graphics::Context->PSSetSamplers(1, 1, shadowSampler.GetAddressOf());
		Graphics::Context->OMSetRenderTargets(1, ppRTV.GetAddressOf(), Graphics::DepthBufferDSV.Get());

		//A5
		// Draw each entity with its own matrix
		for (auto& entity : entities)
		{
			// get the materials
			auto mat = entity.GetMaterial();
			// set the shaders for this entity's material
			Graphics::Context->VSSetShader(mat->GetVertexShader().Get(), 0, 0);
			Graphics::Context->PSSetShader(mat->GetPixelShader().Get(), 0, 0);

			// Build constant buffer data for this specific entity
			VertexShaderExternalData cbData = {};
			cbData.world = entity.GetTransform()->GetWorldMatrix();
			//cbData.colorTint = mat->GetColorTint();
			cbData.worldInvTranspose = entity.GetTransform()->GetWorldInverseTransposeMatrix();
			cbData.view = view;
			cbData.projection = projection;
			cbData.lightView = lightViewMatrix;
			cbData.lightProjection = lightProjectionMatrix;

			// Map, copy, unmap
			//D3D11_MAPPED_SUBRESOURCE mapped = {};
			//Graphics::Context->Map(vsConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
			//memcpy(mapped.pData, &cbData, sizeof(VertexShaderExternalData));
			//Graphics::Context->Unmap(vsConstantBuffer.Get(), 0);

			// builf pixel shader constant buffer data
			PixelShaderExternalData psData = {};
			psData.colorTint = entity.GetMaterial()->GetColorTint();
			psData.uvScale = entity.GetMaterial()->GetUVScale();
			psData.uvOffset = entity.GetMaterial()->GetUVOffset();
			psData.ambientColor = ambientColor;
			psData.cameraPosition = cameras[activeCameraIndex]->GetTransform().GetPosition();
			entity.GetMaterial()->BindTexturesAndSamplers(Graphics::Context);

			// SET LIGHTING STUFF
			//memcpy(&psData.dirLight1, &directionalLight1, sizeof(Light)); 
			memcpy(&psData.lights, &lights[0], sizeof(Light) * (int)lights.size());

			/*D3D11_MAPPED_SUBRESOURCE psMapped = {};
			Graphics::Context->Map(psConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &psMapped);
			memcpy(psMapped.pData, &psData, sizeof(PixelShaderExternalData));
			Graphics::Context->Unmap(psConstantBuffer.Get(), 0);*/

			/*Graphics::Context->VSSetConstantBuffers(0, 1, vsConstantBuffer.GetAddressOf());
			Graphics::Context->PSSetConstantBuffers(0, 1, psConstantBuffer.GetAddressOf());*/

			cbData.worldInvTranspose = entity.GetTransform()->GetWorldInverseTransposeMatrix();

			FillAndBindNextConstantBuffer(&cbData, sizeof(VertexShaderExternalData), true, 0);
			FillAndBindNextConstantBuffer(&psData, sizeof(PixelShaderExternalData), false, 0);

			// Draw the entity (sets VB/IB and calls DrawIndexed)
			entity.Draw(Graphics::Context);
		}

		ID3D11ShaderResourceView* nullSrv[16] = {};
		Graphics::Context->PSSetShaderResources(0, 16, nullSrv);

		//draw the sky
		sky->Draw(cameras[activeCameraIndex]);

		// Post-process chain -------------------------------------------------
		// Source always starts as ppSRV (the off-screen scene).
		// Each active pass reads from one RT and writes to the next.
		// Final pass must write to the back buffer.
		{
			bool doBlur = blurEnabled && (blurRadius > 0);
			bool doChroma = chromaEnabled && (chromaStrength > 0.0f);

			// Determine routing:
			//   blur → ping, chroma → back buffer   (both active)
			//   blur → back buffer                  (blur only)
			//   chroma → back buffer                (chroma only)
			//   neither → blit scene → back buffer  (identity copy)

			if (doBlur)
			{
				BlurCB blurData = {};
				blurData.blurRadius = blurRadius;
				blurData.texelSizeX = 1.0f / (float)Window::Width();
				blurData.texelSizeY = 1.0f / (float)Window::Height();

				ID3D11RenderTargetView* dst = doChroma
					? ppPingRTV.Get()
					: Graphics::BackBufferRTV.Get();

				RunPostProcessPass(blurPS.Get(), ppSRV.Get(), dst,
					&blurData, sizeof(blurData));
			}

			if (doChroma)
			{
				ChromaCB chromaData = {};
				chromaData.strength = chromaStrength;
				chromaData.texelSizeX = 1.0f / (float)Window::Width();
				chromaData.texelSizeY = 1.0f / (float)Window::Height();

				// Read from ping if blur ran first, else read from original scene
				ID3D11ShaderResourceView* src = doBlur
					? ppPingSRV.Get()
					: ppSRV.Get();

				RunPostProcessPass(chromaPS.Get(), src,
					Graphics::BackBufferRTV.Get(),
					&chromaData, sizeof(chromaData));
			}

			// Neither effect active → copy scene to back buffer unchanged
			if (!doBlur && !doChroma)
			{
				BlurCB blurData = {};   // radius 0 → identity copy
				blurData.blurRadius = 0;
				blurData.texelSizeX = 1.0f / (float)Window::Width();
				blurData.texelSizeY = 1.0f / (float)Window::Height();

				RunPostProcessPass(blurPS.Get(), ppSRV.Get(),
					Graphics::BackBufferRTV.Get(),
					&blurData, sizeof(blurData));
			}
		}

		ImGui::Render(); // Turns this frame¡¦s UI into renderable triangles
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData()); // Draws it to the screen
		
		//A4
		// present
		/*bool vsync = Graphics::VsyncState();
		Graphics::SwapChain->Present(vsync ? 1 : 0, vsync ? 0 : DXGI_PRESENT_ALLOW_TEARING);
		Graphics::Context->OMSetRenderTargets(1,
			Graphics::BackBufferRTV.GetAddressOf(),
			Graphics::DepthBufferDSV.Get());*/
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



