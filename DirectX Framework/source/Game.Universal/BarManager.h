#pragma once

#include "DrawableGameComponent.h"
#include <DirectXMath.h>
#include <vector>

namespace DirectXGame
{
	class Bar;
	class Field;

	class BarManager final : public DX::DrawableGameComponent
	{
	public:
		BarManager(const std::shared_ptr<DX::DeviceResources>& deviceResources, const std::shared_ptr<DX::Camera>& camera);

		std::shared_ptr<Field> ActiveField() const;
		void SetActiveField(const std::shared_ptr<Field>& field);

		virtual void CreateDeviceDependentResources() override;
		virtual void ReleaseDeviceDependentResources() override;
		virtual void Update(const DX::StepTimer& timer) override;
		virtual void Render(const DX::StepTimer& timer) override;

		void MoveRight();
		void MoveLeft();

		float HandleBallCollision(const DirectX::XMFLOAT2& ballPosition, const float& ballRadius, float& ballXVelocity);
		bool HandlePowerupCollision(const DirectX::XMFLOAT2& powerupPosition, const float& powerupWidth);

		const std::int32_t BarUpperY() const;
		const std::int32_t BarLowerY() const;

		void IncreaseBarVelocity();
		void DecreaseBarVelocity();

	private:
		void InitializeTriangleVertices();
		void InitializeBar();
		void DrawBar(const Bar& bar);

		static const std::uint32_t CircleResolution;
		static const std::uint32_t SolidCircleVertexCount;

		Microsoft::WRL::ComPtr<ID3D11VertexShader> mVertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> mPixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> mInputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mTriangleVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mVSCBufferPerObject;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mPSCBufferPerObject;

		bool mLoadingComplete;
		std::shared_ptr<Bar> mBar;
		std::shared_ptr<Field> mActiveField;

		const std::int32_t mBarY = 15;
		const std::int32_t mBarHeight = 2;
		const std::int32_t mBarWidth = 8;
	};
}





//#pragma once
//
//#include "DrawableGameComponent.h"
//#include <DirectXMath.h>
//#include <vector>
//
//namespace DX
//{
//	struct VertexPosition;
//}
//
//namespace DirectXGame
//{
//	class Bar;
//
//	class BarManager final : public DX::DrawableGameComponent
//	{
//	public:
//		BarManager(const std::shared_ptr<DX::DeviceResources>& deviceResources, const std::shared_ptr<DX::Camera>& camera);
//
//		std::shared_ptr<Field> ActiveField() const;
//		void SetActiveField(const std::shared_ptr<Field>& field);
//
//		virtual void CreateDeviceDependentResources() override;
//		virtual void ReleaseDeviceDependentResources() override;
//		virtual void Update(const DX::StepTimer& timer) override;
//		virtual void Render(const DX::StepTimer& timer) override;
//
//		void MoveRight();
//		void MoveLeft();
//
//	private:
//		void InitializeTriangleVertices();
//
//		void DrawBar(const Bar& bar);
//
//		static const std::uint32_t SolidCircleVertexCount;
//
//		Microsoft::WRL::ComPtr<ID3D11VertexShader> mVertexShader;
//		Microsoft::WRL::ComPtr<ID3D11PixelShader> mPixelShader;
//		Microsoft::WRL::ComPtr<ID3D11InputLayout> mInputLayout;
//		Microsoft::WRL::ComPtr<ID3D11Buffer> mVertexBuffer;
//		Microsoft::WRL::ComPtr<ID3D11Buffer> mIndexBuffer;
//		Microsoft::WRL::ComPtr<ID3D11Buffer> mVSCBufferPerObject;
//		Microsoft::WRL::ComPtr<ID3D11Buffer> mPSCBufferPerObject;
//
//		Microsoft::WRL::ComPtr<ID3D11Buffer> mTriangleVertexBuffer;
//
//		bool mLoadingComplete;
//		std::uint32_t mIndexCount;
//		std::shared_ptr<Field> mActiveField;
//		std::shared_ptr<Bar> mBar;
//	};
//}