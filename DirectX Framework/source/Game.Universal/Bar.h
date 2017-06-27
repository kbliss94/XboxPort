#pragma once

#include "Transform2D.h"
#include "VectorHelper.h"
#include <DirectXMath.h>
#include <DirectXColors.h>

namespace DX
{
	class StepTimer;
}

namespace DirectXGame
{
	class BarManager;

	class Bar final
	{
	public:
		Bar(BarManager& barManager, const DX::Transform2D& transform, float radius, const DirectX::XMFLOAT4& color, const DirectX::XMFLOAT2& velocity);
		Bar(const Bar&) = default;
		Bar& operator=(const Bar&) = delete;
		Bar(Bar&&) = default;
		Bar& operator=(Bar&&) = default;
		~Bar() = default;

		const DX::Transform2D& Transform() const;
		void SetTransform(const DX::Transform2D& transform);

		const DirectX::XMFLOAT2& Position() const;
		const float& Radius() const;
		const float& Width() const;

		const DirectX::XMFLOAT4& Color() const;
		void SetColor(const DirectX::XMFLOAT4& color);

		const DirectX::XMFLOAT2& Velocity() const;
		void SetVelocity(const DirectX::XMFLOAT2& velocity);

		void Update(const DX::StepTimer& timer);

	private:
		void CheckForFieldCollision();

		BarManager& mBarManager;
		DX::Transform2D mTransform;
		float mRadius;
		DirectX::XMFLOAT4 mColor;
		DirectX::XMFLOAT2 mVelocity;

		const float mWidth = 4.0f;
		const float mFieldRightSide = 40.0f;
		const float mFieldLeftSide = -52.0f;
	};
}





//#pragma once
//
//#include <DirectXColors.h>
//#include <SimpleMath.h>
//
//namespace DirectXGame
//{
//	class BarManager;
//
//	class Bar final : public Field
//	{
//	public:
//		Bar(const DirectX::XMFLOAT2& position, const DirectX::XMFLOAT2& size, const DirectX::XMFLOAT4& color,
//			const DirectX::XMFLOAT2& velocity, BarManager& barManager);
//
//		void Update(const DX::StepTimer& timer);
//
//		const DirectX::XMFLOAT2& Velocity() const;
//		void SetVelocity(const DirectX::XMFLOAT2& velocity);
//
//	private:
//		void CheckForFieldCollision();
//
//		DirectX::XMFLOAT2 mVelocity;
//		BarManager& mBarManager;
//
//		const float mWidth = 10.0f;
//	};
//}