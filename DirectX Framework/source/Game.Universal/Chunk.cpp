#include "pch.h"
#include "Chunk.h"
#include "ChunkManager.h"

using namespace DirectX;

namespace DirectXGame
{
	Chunk::Chunk(const DirectX::XMFLOAT2& position, const DirectX::XMFLOAT2& size, const DirectX::XMFLOAT4& color, 
		ChunkManager& chunkManager) :
		Field(position, size, color), mChunkManager(chunkManager)
	{

	}

	//void Chunk::Update(const DX::StepTimer& timer)
	//{
	//	double elapsedTime = timer.GetElapsedSeconds();

	//	mPosition.x += mVelocity.x * static_cast<float>(elapsedTime);
	//	mPosition.y += mVelocity.y * static_cast<float>(elapsedTime);

	//	SetPosition(mPosition);

	//	CheckForFieldCollision();
	//}
}