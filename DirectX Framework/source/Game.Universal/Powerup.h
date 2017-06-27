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
	class PowerupManager;

	class Powerup
	{
	public:

		enum PowerupType
		{
			FasterBall,
			SlowerBall,
			SlowerBar,
			FasterBar
		};

		Powerup(PowerupManager& powerupManager, const DX::Transform2D& transform, float radius, const DirectX::XMFLOAT4& color, 
			const DirectX::XMFLOAT2& velocity, PowerupType type);
		Powerup(const Powerup&) = default;
		Powerup& operator=(const Powerup&) = delete;
		Powerup(Powerup&&) = default;
		Powerup& operator=(Powerup&&) = default;
		~Powerup() = default;

		const DX::Transform2D& Transform() const;
		void SetTransform(const DX::Transform2D& transform);

		const DirectX::XMFLOAT2& Position() const;
		const float& Radius() const;
		const PowerupType Type() const;

		const DirectX::XMFLOAT4& Color() const;
		void SetColor(const DirectX::XMFLOAT4& color);

		const DirectX::XMFLOAT2& Velocity() const;
		void SetVelocity(const DirectX::XMFLOAT2& velocity);

		void Update(const DX::StepTimer& timer);

		void ActivatePowerup();
		const bool Activated() const;

	private:
		void CheckForFieldCollision();

		PowerupManager& mPowerupManager;
		DX::Transform2D mTransform;
		float mRadius;
		DirectX::XMFLOAT4 mColor;
		DirectX::XMFLOAT2 mVelocity;

		const float mWidth = 3.0f;
		const float mFieldRightSide = 40.0f;
		const float mFieldLeftSide = -52.0f;

		PowerupType mType;
		bool mActivated;
	};
}
