#include "pch.h"
#include "Bar.h"

using namespace DirectX;
using namespace DX;

namespace DirectXGame
{
	Bar::Bar(BarManager& barManager, const DX::Transform2D& transform, float radius, const DirectX::XMFLOAT4& color, const DirectX::XMFLOAT2& velocity) :
		mBarManager(barManager), mTransform(transform), mRadius(radius),
		mColor(color), mVelocity(velocity)
	{
	}

	const Transform2D& Bar::Transform() const
	{
		return mTransform;
	}

	void Bar::SetTransform(const Transform2D & transform)
	{
		mTransform = transform;
	}

	const XMFLOAT2& Bar::Position() const
	{
		return mTransform.Position();
	}

	const float& Bar::Radius() const
	{
		return mRadius;
	}

	const float& Bar::Width() const
	{
		return mWidth;
	}

	const XMFLOAT4& Bar::Color() const
	{
		return mColor;
	}

	void Bar::SetColor(const XMFLOAT4& color)
	{
		mColor = color;
	}

	const XMFLOAT2& Bar::Velocity() const
	{
		return mVelocity;
	}

	void Bar::SetVelocity(const XMFLOAT2& velocity)
	{
		mVelocity = velocity;
	}

	void Bar::Update(const StepTimer& timer)
	{
		double elapsedTime = timer.GetElapsedSeconds();

		XMFLOAT2 position = mTransform.Position();
		position.x += mVelocity.x * static_cast<float>(elapsedTime);
		position.y += mVelocity.y * static_cast<float>(elapsedTime);

		mTransform.SetPosition(position);
		CheckForFieldCollision();
	}

	void Bar::CheckForFieldCollision()
	{
		const auto& position = mTransform.Position();
		XMFLOAT2 updatedPosition = position;
		bool hasCollidedWithField = false;

		if (position.x - mWidth <= mFieldLeftSide)
		{
			updatedPosition.x = mFieldLeftSide + mWidth;
			hasCollidedWithField = true;
		}

		if (position.x + mWidth >= mFieldRightSide)
		{
			updatedPosition.x = mFieldRightSide - mWidth;
			hasCollidedWithField = true;
		}

		if (hasCollidedWithField)
		{
			mTransform.SetPosition(updatedPosition);
		}
	}
}






//#include "pch.h"
//#include "Bar.h"
//#include "BarManager.h"
//
//using namespace DirectX;
//
//namespace DirectXGame
//{
//	Bar::Bar(const DirectX::XMFLOAT2& position, const DirectX::XMFLOAT2& size, const DirectX::XMFLOAT4& color,
//		const DirectX::XMFLOAT2& velocity, BarManager& barManager) :
//		Field(position, size, color), mVelocity(velocity), mBarManager(barManager)
//	{
//
//	}
//
//	void Bar::Update(const DX::StepTimer& timer)
//	{
//		double elapsedTime = timer.GetElapsedSeconds();
//
//		mPosition.x += mVelocity.x * static_cast<float>(elapsedTime);
//		mPosition.y += mVelocity.y * static_cast<float>(elapsedTime);
//
//		SetPosition(mPosition);
//
//		CheckForFieldCollision();
//	}
//
//	const DirectX::XMFLOAT2& Bar::Velocity() const
//	{
//		return mVelocity;
//	}
//
//	void Bar::SetVelocity(const DirectX::XMFLOAT2& velocity)
//	{
//		mVelocity = velocity;
//	}
//
//	void Bar::CheckForFieldCollision()
//	{
//		//don't need to worry about top side collision or bottom side i don't think
//		//also upon collision of right side or left side need to reverse direction 
//			//prob just multiply velocity by -1
//
//		auto field = mBarManager.ActiveField();
//		const auto& fieldPosition = field->Position();
//		const auto& fieldSize = field->Size();
//		const XMFLOAT2 fieldHalfSize(fieldSize.x / 2.0f, fieldSize.y / 2.0f);
//
//		const float rightSide = fieldPosition.x + fieldHalfSize.x;
//		const float leftSide = fieldPosition.x - fieldHalfSize.x;
//		const float topSide = fieldPosition.y + fieldHalfSize.y;
//		const float bottomSide = fieldPosition.y - fieldHalfSize.y;
//
//		XMFLOAT2 updatedPosition = mPosition;
//
//		bool hasCollidedWithField = false;
//		if (mPosition.x - mWidth <= leftSide)
//		{
//			//mVelocity.x *= -1;
//			updatedPosition.x = leftSide + mWidth;
//			hasCollidedWithField = true;
//		}
//		if (mPosition.x + mWidth >= rightSide)
//		{
//			//mVelocity.x *= -1;
//			updatedPosition.x = rightSide - mWidth;
//			hasCollidedWithField = true;
//		}
//		//if (mPosition.y - mWidth <= bottomSide)
//		//{
//		//	mVelocity.y *= -1;
//		//	updatedPosition.y = bottomSide + mWidth;
//		//	hasCollidedWithField = true;
//		//}
//		//if (mPosition.y + mWidth >= topSide)
//		//{
//		//	mVelocity.y *= -1;
//		//	updatedPosition.y = topSide - mWidth;
//		//	hasCollidedWithField = true;
//		//}
//
//		if (hasCollidedWithField)
//		{
//			SetPosition(updatedPosition);
//		}
//	}
//}