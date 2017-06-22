#pragma once

#include <DirectXColors.h>
#include <SimpleMath.h>

namespace DirectXGame
{
	class BarManager;

	class Bar final : public Field
	{
	public:
		Bar(const DirectX::XMFLOAT2& position, const DirectX::XMFLOAT2& size, const DirectX::XMFLOAT4& color, 
			const DirectX::XMFLOAT2& velocity, BarManager& barManager);
	
		void Update(const DX::StepTimer& timer);

		const DirectX::XMFLOAT2& Velocity() const;
		void SetVelocity(const DirectX::XMFLOAT2& velocity);

	private:
		void CheckForFieldCollision();

		DirectX::XMFLOAT2 mVelocity;
		BarManager& mBarManager;

		const float mWidth = 10.0f;
	};
}