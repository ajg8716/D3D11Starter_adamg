#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <string>
#include "BufferStructs.h"
#include <vector>
#include <memory>
#include "Mesh.h"
#include "GameEntity.h"
#include "Camera.h"
#include "Material.h"
#include "WICTextureLoader.h"
#include "Lights.h"
#include "Sky.h"

// -- Post Process constant buffer structs ------------------------------
// must be 16-byte aligned
struct BlurCB
{
	int blurRadius;
	float texelSizeX;
	float texelSizeY;
	float padding;
};

struct ChromaCB 
{
	float strength;
	float texelSizeX;
	float texelSizeY;
	float padding;
};

class Game
{
public:
	// Basic OOP setup
	Game();
	~Game();
	Game(const Game&) = delete; // Remove copy constructor
	Game& operator=(const Game&) = delete; // Remove copy-assignment operator

	DirectX::XMFLOAT3 ambientColor;
	std::vector<Light> lights;
	std::shared_ptr<Sky> sky;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvCobblestoneNormalMap;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvBronzeNormalMap;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvFloorNormalMap;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvFlatNormalMap;

	// Primary functions
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);
	void OnResize();

private:

	// constant buffer
	/*Microsoft::WRL::ComPtr<ID3D11Buffer> vsConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> psConstantBuffer;*/

	// Textures & sampler
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvCobblestoneTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvBronzeTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvFloorTexture;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;

	// Ring buffer constant buffer
	Microsoft::WRL::ComPtr<ID3D11Buffer> cbRingBuffer;
	size_t cbRingBufferOffset = 0;
	static const size_t cbRingBufferSize = 256000;

	// Helper method
	void FillAndBindNextConstantBuffer(void* data, size_t dataSize,
		bool isVertexShader, UINT slot);


	// UI-editable data
	DirectX::XMFLOAT4 colorTint;
	//DirectX::XMFLOAT3 offset;

	// Initialization helper methods - feel free to customize, combine, remove, etc.
	void LoadShaders();
	void CreateGeometry();
	void ImGuiFresh(float);
	void CreateShadowMapResources();	
	void RenderShadowMap();
	void CreatePostProcessResources();
	void RunPostProcessPass(ID3D11PixelShader* ps,
		ID3D11ShaderResourceView* srcSRV,
		ID3D11RenderTargetView* dstRTV,
		void* cbData,
		size_t                        cbSize);

	//mesh storage - using shared_ptr for proper lifetime management
	std::vector<std::shared_ptr<Mesh>> meshes;

	std::vector<std::shared_ptr<Material>> materials;

	// game entitites
	std::vector<GameEntity> entities;

	// Cameras 
	std::vector<std::shared_ptr<Camera>> cameras;
	int activeCameraIndex; // tracks which camera is currently active


	//establish color as a member of the Game class
	float color[4] = { 0.4f, 0.6f, 0.75f, 0.0f };

	// UI state
	bool showDemoMenu = false;
	bool showHappyMeter = false;
	int number = 0;
	std::string happyMeterMessage = "happiness lvl???";

	// Note the usage of ComPtr below
	//  - This is a smart pointer for objects that abide by the
	//     Component Object Model, which DirectX objects do
	//  - More info here: https://github.com/Microsoft/DirectXTK/wiki/ComPtr

	// Buffers to hold actual geometry data
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

	// Shaders and shader-related constructs
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

	// shadow resources
	Microsoft::WRL::ComPtr<ID3D11VertexShader> shadowVertexShader;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> shadowDSV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shadowSRV;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> shadowRasterizer;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> shadowSampler;
	DirectX::XMFLOAT4X4 lightViewMatrix;
	DirectX::XMFLOAT4X4 lightProjectionMatrix;

	// ---- Post-process resources ----------------------------------------------
	// Off-screen render target: all 3-D rendering goes here first
	Microsoft::WRL::ComPtr<ID3D11Texture2D>          ppTexture;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView>   ppRTV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ppSRV;

	// Intermediate (ping-pong) target used when both effects are active
	Microsoft::WRL::ComPtr<ID3D11Texture2D>          ppPingTexture;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView>   ppPingRTV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ppPingSRV;

	// Clamp sampler shared by all post-process passes
	Microsoft::WRL::ComPtr<ID3D11SamplerState>       ppSampler;

	// Post-process shaders
	Microsoft::WRL::ComPtr<ID3D11VertexShader> ppVS;     // full-screen triangle VS
	Microsoft::WRL::ComPtr<ID3D11PixelShader>  blurPS;   // Task 1
	Microsoft::WRL::ComPtr<ID3D11PixelShader>  chromaPS; // Task 2

	// ImGui-controlled post-process parameters
	bool  blurEnabled = true;
	int   blurRadius = 0;        // 0 = no blur, max slider = 10
	bool  chromaEnabled = true;
	float chromaStrength = 0.005f;   // 0 = no effect, max slider = 0.03
	// ----------------------------------------------------------------------------
};

