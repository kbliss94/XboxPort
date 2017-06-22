#include "pch.h"
#include "Bar.h"
#include "BarManager.h"

using namespace DirectX;

namespace DirectXGame
{
	Bar::Bar(const DirectX::XMFLOAT2& position, const DirectX::XMFLOAT2& size, const DirectX::XMFLOAT4& color,
		const DirectX::XMFLOAT2& velocity, BarManager& barManager) :
		Field(position, size, color), mVelocity(velocity), mBarManager(barManager)
	{

	}

	void Bar::Update(const DX::StepTimer& timer)
	{
		double elapsedTime = timer.GetElapsedSeconds();

		mPosition.x += mVelocity.x * static_cast<float>(elapsedTime);
		mPosition.y += mVelocity.y * static_cast<float>(elapsedTime);

		SetPosition(mPosition);

		CheckForFieldCollision();
	}

	const DirectX::XMFLOAT2& Bar::Velocity() const
	{
		return mVelocity;
	}

	void Bar::SetVelocity(const DirectX::XMFLOAT2& velocity)
	{
		mVelocity = velocity;
	}

	void Bar::CheckForFieldCollision()
	{
		//don't need to worry about top side collision or bottom side i don't think
		//also upon collision of right side or left side need to reverse direction 
			//prob just multiply velocity by -1

		auto field = mBarManager.ActiveField();
		const auto& fieldPosition = field->Position();
		const auto& fieldSize = field->Size();
		const XMFLOAT2 fieldHalfSize(fieldSize.x / 2.0f, fieldSize.y / 2.0f);

		const float rightSide = fieldPosition.x + fieldHalfSize.x;
		const float leftSide = fieldPosition.x - fieldHalfSize.x;
		const float topSide = fieldPosition.y + fieldHalfSize.y;
		const float bottomSide = fieldPosition.y - fieldHalfSize.y;

		XMFLOAT2 updatedPosition = mPosition;

		bool hasCollidedWithField = false;
		if (mPosition.x - mWidth <= leftSide)
		{
			//mVelocity.x *= -1;
			updatedPosition.x = leftSide + mWidth;
			hasCollidedWithField = true;
		}
		if (mPosition.x + mWidth >= rightSide)
		{
			//mVelocity.x *= -1;
			updatedPosition.x = rightSide - mWidth;
			hasCollidedWithField = true;
		}
		//if (mPosition.y - mWidth <= bottomSide)
		//{
		//	mVelocity.y *= -1;
		//	updatedPosition.y = bottomSide + mWidth;
		//	hasCollidedWithField = true;
		//}
		//if (mPosition.y + mWidth >= topSide)
		//{
		//	mVelocity.y *= -1;
		//	updatedPosition.y = topSide - mWidth;
		//	hasCollidedWithField = true;
		//}

		if (hasCollidedWithField)
		{
			SetPosition(updatedPosition);
		}
	}
}