#pragma once

#include <DirectXColors.h>
#include <SimpleMath.h>

namespace DirectXGame
{
	class Field final
	{
	public:
		Field(const DirectX::XMFLOAT2& position, const DirectX::XMFLOAT2& size, const DirectX::XMFLOAT4& color = DirectX::XMFLOAT4(&DirectX::Colors::White[0]));

		const DirectX::XMFLOAT2& Position() const;
		void SetPosition(const DirectX::XMFLOAT2& position);

		const DirectX::XMFLOAT2& Size() const;
		void SetSize(const DirectX::XMFLOAT2& size);

		const DirectX::XMFLOAT4& Color() const;
		void SetColor(const DirectX::XMFLOAT4& color);

	private:
		DirectX::XMFLOAT2 mPosition;
		DirectX::XMFLOAT2 mSize;
		DirectX::XMFLOAT4 mColor;

		//actually this would be constant/the same for every field chunk
		//DirectX::XMFLOAT2 mHeight;
		//DirectX::XMFLOAT2 mWidth;
	};
}

