#pragma once

#include "Powerup.h"

#include "DrawableGameComponent.h"
#include <DirectXMath.h>
#include <vector>
#include <DirectXColors.h>

namespace DirectXGame
{
	//class Powerup;
	class Field;
	class BarManager;
	class BallManager;

	class PowerupManager final : public DX::DrawableGameComponent
	{
	public:
		struct PowerupData
		{
			DirectX::XMFLOAT4 Color;
			Powerup::PowerupType Type;

			PowerupData(const DirectX::XMFLOAT4& color, const Powerup::PowerupType type) :
				Color(color), Type(type)
			{
			};
		};

		PowerupManager(const std::shared_ptr<DX::DeviceResources>& deviceResources, const std::shared_ptr<DX::Camera>& camera,
			BarManager& barManager);

		std::shared_ptr<Field> ActiveField() const;
		void SetActiveField(const std::shared_ptr<Field>& field);

		virtual void CreateDeviceDependentResources() override;
		virtual void ReleaseDeviceDependentResources() override;
		virtual void Update(const DX::StepTimer& timer) override;
		virtual void Render(const DX::StepTimer& timer) override;

		//returns the y position at which a chunk was hit & deleted (returns 0 if no chunk was hit)
		//float HandleBarCollision(const DirectX::XMFLOAT2& ballPosition, const float& ballRadius);

		//could pass in the hit chunk's position to see where to spawn the powerup
		void PowerupSpawnCheck(const DirectX::XMFLOAT2& chunkPosition);

		void SetBallManager(std::shared_ptr<BallManager> ballManager);

	private:
		void SpawnPowerup(const DirectX::XMFLOAT2& chunkPosition);

		void InitializeTriangleVertices();
		void InitializePowerup(const DirectX::XMFLOAT2& position, Powerup::PowerupType type, const DirectX::XMFLOAT4& color);
		void DrawPowerup(const Powerup& powerup);

		static const std::uint32_t CircleResolution;
		static const std::uint32_t SolidCircleVertexCount;

		Microsoft::WRL::ComPtr<ID3D11VertexShader> mVertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> mPixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> mInputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mTriangleVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mVSCBufferPerObject;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mPSCBufferPerObject;

		bool mLoadingComplete;
		std::vector<std::shared_ptr<Powerup>> mPowerups;
		std::shared_ptr<Field> mActiveField;

		const int32_t mPowerupHeight = 2;
		const float mPowerupWidth = 3.0f;

		const std::vector <DirectX::XMFLOAT4> mChunkColors =
		{
			(DirectX::XMFLOAT4)DirectX::Colors::HotPink,
			(DirectX::XMFLOAT4)DirectX::Colors::Red,
			(DirectX::XMFLOAT4)DirectX::Colors::Orange,
			(DirectX::XMFLOAT4)DirectX::Colors::Yellow,
			(DirectX::XMFLOAT4)DirectX::Colors::LawnGreen,
			(DirectX::XMFLOAT4)DirectX::Colors::LightSkyBlue
		};

		const std::vector <PowerupData> mPossiblePowerups =
		{
			PowerupData((DirectX::XMFLOAT4)DirectX::Colors::MediumPurple, Powerup::PowerupType::FasterBall),
			PowerupData((DirectX::XMFLOAT4)DirectX::Colors::MistyRose, Powerup::PowerupType::SlowerBall),
			PowerupData((DirectX::XMFLOAT4)DirectX::Colors::PowderBlue, Powerup::PowerupType::SlowerBar),
			PowerupData((DirectX::XMFLOAT4)DirectX::Colors::Purple, Powerup::PowerupType::FasterBar)
		};

		BarManager& mBarManager;
		std::shared_ptr<BallManager> mBallManager;
	};
}