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
	class ChunkManager;

	class Chunk final
	{
	public:
		Chunk(ChunkManager& chunkManager, const DX::Transform2D& transform, float radius, const DirectX::XMFLOAT4& color, const DirectX::XMFLOAT2& velocity);
		Chunk(const Chunk&) = default;
		Chunk& operator=(const Chunk&) = delete;
		Chunk(Chunk&&) = default;
		Chunk& operator=(Chunk&&) = default;
		~Chunk() = default;

		const DX::Transform2D& Transform() const;
		void SetTransform(const DX::Transform2D& transform);

		const DirectX::XMFLOAT2& Position() const;
		const float& Radius() const;

		const DirectX::XMFLOAT4& Color() const;
		void SetColor(const DirectX::XMFLOAT4& color);

		const DirectX::XMFLOAT2& Velocity() const;
		void SetVelocity(const DirectX::XMFLOAT2& velocity);

		void Update(const DX::StepTimer& timer);

	private:
		ChunkManager& mChunkManager;
		DX::Transform2D mTransform;
		float mRadius;
		DirectX::XMFLOAT4 mColor;
		DirectX::XMFLOAT2 mVelocity;

		const float mWidth = 4.0f;
		const float mFieldRightSide = 40.0f;
		const float mFieldLeftSide = -52.0f;
	};
}
