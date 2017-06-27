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
	class BallManager;
	class ChunkManager;
	class BarManager;

	class Ball final
	{
	public:
		Ball(BallManager& ballManager, ChunkManager& chunkManager, BarManager& barManager, const DX::Transform2D& transform, float radius,
			const DirectX::XMFLOAT4& color = DirectX::XMFLOAT4(&DirectX::Colors::White[0]), const DirectX::XMFLOAT2& velocity = DX::Vector2Helper::Zero);
		Ball(const Ball&) = default;
		Ball& operator=(const Ball&) = delete;
		Ball(Ball&&) = default;
		Ball& operator=(Ball&&) = default;
		~Ball() = default;

		const DX::Transform2D& Transform() const;
		void SetTransform(const DX::Transform2D& transform);

		float Radius() const;

		const DirectX::XMFLOAT4& Color() const;
		void SetColor(const DirectX::XMFLOAT4& color);

		const DirectX::XMFLOAT2& Velocity() const;
		void SetVelocity(const DirectX::XMFLOAT2& velocity);

		void Update(const DX::StepTimer& timer);

	private:
		void CheckForFieldCollision();

		BallManager& mBallManager;
		DX::Transform2D mTransform;
		float mRadius;
		DirectX::XMFLOAT4 mColor;
		DirectX::XMFLOAT2 mVelocity;

		ChunkManager& mChunkManager;
		BarManager& mBarManager;
	};
}