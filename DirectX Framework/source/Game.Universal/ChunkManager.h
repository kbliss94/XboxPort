#pragma once

#include "DrawableGameComponent.h"
#include <DirectXMath.h>
#include <vector>
#include <DirectXColors.h>

namespace DirectXGame
{
	class Chunk;
	class Field;
	class ScoreManager;
	class PowerupManager;

	class ChunkManager final : public DX::DrawableGameComponent
	{
	public:
		ChunkManager(const std::shared_ptr<DX::DeviceResources>& deviceResources, const std::shared_ptr<DX::Camera>& camera, 
			ScoreManager& scoreManager, PowerupManager& powerupManager);

		std::shared_ptr<Field> ActiveField() const;
		void SetActiveField(const std::shared_ptr<Field>& field);

		virtual void CreateDeviceDependentResources() override;
		virtual void ReleaseDeviceDependentResources() override;
		virtual void Update(const DX::StepTimer& timer) override;
		virtual void Render(const DX::StepTimer& timer) override;

		float HandleBallCollision(const DirectX::XMFLOAT2& ballPosition, const float& ballRadius);
		void GameOver();

	private:
		void InitializeTriangleVertices();
		void InitializeChunks();
		void DrawChunk(const Chunk& chunk);

		static const std::uint32_t CircleResolution;
		static const std::uint32_t SolidCircleVertexCount;

		Microsoft::WRL::ComPtr<ID3D11VertexShader> mVertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> mPixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> mInputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mTriangleVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mVSCBufferPerObject;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mPSCBufferPerObject;

		bool mLoadingComplete;
		std::vector<std::shared_ptr<Chunk>> mChunks;
		std::shared_ptr<Field> mActiveField;
		ScoreManager& mScoreManager;
		PowerupManager& mPowerupManager;

		const uint32_t mNumChunks = 60;
		const int32_t mChunkHeight = 3;
		const float mChunkWidth = 9.0f;

		const std::vector <DirectX::XMFLOAT4> mChunkColors =
		{
			(DirectX::XMFLOAT4)DirectX::Colors::HotPink,
			(DirectX::XMFLOAT4)DirectX::Colors::Red, 
			(DirectX::XMFLOAT4)DirectX::Colors::Orange, 
			(DirectX::XMFLOAT4)DirectX::Colors::Yellow, 
			(DirectX::XMFLOAT4)DirectX::Colors::LawnGreen,
			(DirectX::XMFLOAT4)DirectX::Colors::LightSkyBlue
		};
	};
}