#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <string>
#include "BufferStructs.h"
#include <vector>
#include <memory>
#include "Mesh.h"
#include "GameEntity.h"


class Game
{
public:
	// Basic OOP setup
	Game();
	~Game();
	Game(const Game&) = delete; // Remove copy constructor
	Game& operator=(const Game&) = delete; // Remove copy-assignment operator

	// Primary functions
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);
	void OnResize();

private:

	// constant buffer
	Microsoft::WRL::ComPtr<ID3D11Buffer> vsConstantBuffer;


	// UI-editable data
	DirectX::XMFLOAT4 colorTint;
	//DirectX::XMFLOAT3 offset;

	// Initialization helper methods - feel free to customize, combine, remove, etc.
	void LoadShaders();
	void CreateGeometry();
	void ImGuiFresh(float);

	//mesh storage - using shared_ptr for proper lifetime management
	std::vector<std::shared_ptr<Mesh>> meshes;

	// game entitites
	std::vector<GameEntity> entities;

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
};

