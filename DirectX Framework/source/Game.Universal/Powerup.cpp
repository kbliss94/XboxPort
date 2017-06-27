#include "pch.h"
#include "Powerup.h"

using namespace DirectX;
using namespace DX;

namespace DirectXGame
{
	Powerup::Powerup(PowerupManager& powerupManager, const DX::Transform2D& transform, float radius, const DirectX::XMFLOAT4& color, 
		const DirectX::XMFLOAT2& velocity, PowerupType type) :
		mPowerupManager(powerupManager), mTransform(transform), mRadius(radius),
		mColor(color), mVelocity(velocity), mType(type), mActivated(false)
	{
	}

	const Transform2D& Powerup::Transform() const
	{
		return mTransform;
	}

	void Powerup::SetTransform(const Transform2D & transform)
	{
		mTransform = transform;
	}

	const XMFLOAT2& Powerup::Position() const
	{
		return mTransform.Position();
	}

	const float& Powerup::Radius() const
	{
		return mRadius;
	}

	const Powerup::PowerupType Powerup::Type() const
	{
		return mType;
	}

	const XMFLOAT4& Powerup::Color() const
	{
		return mColor;
	}

	void Powerup::SetColor(const XMFLOAT4& color)
	{
		mColor = color;
	}

	const XMFLOAT2& Powerup::Velocity() const
	{
		return mVelocity;
	}

	void Powerup::SetVelocity(const XMFLOAT2& velocity)
	{
		mVelocity = velocity;
	}

	void Powerup::Update(const StepTimer& timer)
	{
		double elapsedTime = timer.GetElapsedSeconds();

		XMFLOAT2 position = mTransform.Position();
		position.x += mVelocity.x * static_cast<float>(elapsedTime);
		position.y += mVelocity.y * static_cast<float>(elapsedTime);

		mTransform.SetPosition(position);
		CheckForFieldCollision();
	}

	void Powerup::ActivatePowerup()
	{
		mActivated = true;
	}

	const bool Powerup::Activated() const
	{
		return mActivated;
	}

	void Powerup::CheckForFieldCollision()
	{
		//bar is always at y=15, so once powerup goes past this,
			//call HandlePowerupCollision in BarManager


		//const auto& position = mTransform.Position();
		//XMFLOAT2 updatedPosition = position;
		//bool hasCollidedWithField = false;

		//if (position.x - mWidth <= mFieldLeftSide)
		//{
		//	updatedPosition.x = mFieldLeftSide + mWidth;
		//	hasCollidedWithField = true;
		//}

		//if (position.x + mWidth >= mFieldRightSide)
		//{
		//	updatedPosition.x = mFieldRightSide - mWidth;
		//	hasCollidedWithField = true;
		//}

		//if (hasCollidedWithField)
		//{
		//	mTransform.SetPosition(updatedPosition);
		//}
	}
}