#pragma once

#include "DrawableGameComponent.h"
#include <DirectXMath.h>
#include <vector>

namespace DirectXGame
{
	class Ball;
	class Field;
	class ChunkManager;
	class BarManager;

	class BallManager final : public DX::DrawableGameComponent
	{
	public:
		BallManager(const std::shared_ptr<DX::DeviceResources>& deviceResources, const std::shared_ptr<DX::Camera>& camera, ChunkManager& chunkManager, BarManager& barManager);

		std::shared_ptr<Field> ActiveField() const;
		void SetActiveField(const std::shared_ptr<Field>& field);

		virtual void CreateDeviceDependentResources() override;
		virtual void ReleaseDeviceDependentResources() override;
		virtual void Update(const DX::StepTimer& timer) override;
		virtual void Render(const DX::StepTimer& timer) override;

		void IncreaseBallVelocity();
		void DecreaseBallVelocity();

		const bool LaunchedBall() const;
		void LaunchBall();
		void BallOffscreen();

	private:
		void InitializeLineVertices();
		void InitializeTriangleVertices();
		void InitializeBall();
		void DrawSolidBall(const Ball& ball);

		static const std::uint32_t CircleResolution;
		static const std::uint32_t LineCircleVertexCount;
		static const std::uint32_t SolidCircleVertexCount;

		Microsoft::WRL::ComPtr<ID3D11VertexShader> mVertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> mPixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> mInputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mLineVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mTriangleVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mVSCBufferPerObject;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mPSCBufferPerObject;

		bool mLoadingComplete;
		bool mBallLaunched;
		std::shared_ptr<Ball> mBall;
		std::shared_ptr<Field> mActiveField;
		ChunkManager& mChunkManager;
		BarManager& mBarManager;

		const DirectX::XMFLOAT2 mInitialVelocity = DirectX::XMFLOAT2(17.0f, 17.0f);
	};
}

