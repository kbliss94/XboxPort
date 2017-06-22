#pragma once

#include <DirectXColors.h>
#include <SimpleMath.h>

namespace DirectXGame
{
	class ChunkManager;

	class Chunk final : public Field
	{
	public:
		Chunk(const DirectX::XMFLOAT2& position, const DirectX::XMFLOAT2& size, const DirectX::XMFLOAT4& color, ChunkManager& chunkManager);

		//void Update(const DX::StepTimer& timer);

	private:
		ChunkManager& mChunkManager;

		const float mWidth = 10.0f;
	};
}
