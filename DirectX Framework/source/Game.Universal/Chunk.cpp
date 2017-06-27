#include "pch.h"
#include "Chunk.h"

using namespace DirectX;
using namespace DX;

namespace DirectXGame
{
	Chunk::Chunk(ChunkManager& chunkManager, const DX::Transform2D& transform, float radius, const DirectX::XMFLOAT4& color, const DirectX::XMFLOAT2& velocity) :
		mChunkManager(chunkManager), mTransform(transform), mRadius(radius),
		mColor(color), mVelocity(velocity)
	{
	}

	const Transform2D& Chunk::Transform() const
	{
		return mTransform;
	}

	void Chunk::SetTransform(const Transform2D & transform)
	{
		mTransform = transform;
	}

	const XMFLOAT2& Chunk::Position() const
	{
		return mTransform.Position();
	}

	const float& Chunk::Radius() const
	{
		return mRadius;
	}

	const XMFLOAT4& Chunk::Color() const
	{
		return mColor;
	}

	void Chunk::SetColor(const XMFLOAT4& color)
	{
		mColor = color;
	}

	const XMFLOAT2& Chunk::Velocity() const
	{
		return mVelocity;
	}

	void Chunk::SetVelocity(const XMFLOAT2& velocity)
	{
		mVelocity = velocity;
	}

	void Chunk::Update(const StepTimer& timer)
	{
		double elapsedTime = timer.GetElapsedSeconds();

		XMFLOAT2 position = mTransform.Position();
		position.x += mVelocity.x * static_cast<float>(elapsedTime);
		position.y += mVelocity.y * static_cast<float>(elapsedTime);

		mTransform.SetPosition(position);
	}
}