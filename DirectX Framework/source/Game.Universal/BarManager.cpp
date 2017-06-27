#include "pch.h"
#include "BarManager.h"
#include "Bar.h"

using namespace std;
using namespace DirectX;
using namespace DX;

namespace DirectXGame
{
	const uint32_t BarManager::CircleResolution = 32;
	const uint32_t BarManager::SolidCircleVertexCount = (BarManager::CircleResolution + 1) * 2;

	BarManager::BarManager(const shared_ptr<DX::DeviceResources>& deviceResources, const shared_ptr<Camera>& camera) :
		DrawableGameComponent(deviceResources, camera), mLoadingComplete(false)
	{
		CreateDeviceDependentResources();
	}

	std::shared_ptr<Field> BarManager::ActiveField() const
	{
		return mActiveField;
	}

	void BarManager::SetActiveField(const shared_ptr<Field>& field)
	{
		mActiveField = field;
	}

	void BarManager::CreateDeviceDependentResources()
	{
		auto loadVSTask = ReadDataAsync(L"ShapeRendererVS.cso");
		auto loadPSTask = ReadDataAsync(L"ShapeRendererPS.cso");

		// After the vertex shader file is loaded, create the shader and input layout.
		auto createVSTask = loadVSTask.then([this](const std::vector<byte>& fileData) {
			ThrowIfFailed(
				mDeviceResources->GetD3DDevice()->CreateVertexShader(
					&fileData[0],
					fileData.size(),
					nullptr,
					mVertexShader.ReleaseAndGetAddressOf()
				)
			);

			// Create an input layout
			ThrowIfFailed(
				mDeviceResources->GetD3DDevice()->CreateInputLayout(
					VertexPosition::InputElements,
					VertexPosition::InputElementCount,
					&fileData[0],
					fileData.size(),
					mInputLayout.ReleaseAndGetAddressOf()
				)
			);

			CD3D11_BUFFER_DESC constantBufferDesc(sizeof(XMFLOAT4X4), D3D11_BIND_CONSTANT_BUFFER);
			ThrowIfFailed(
				mDeviceResources->GetD3DDevice()->CreateBuffer(
					&constantBufferDesc,
					nullptr,
					mVSCBufferPerObject.ReleaseAndGetAddressOf()
				)
			);
		});

		// After the pixel shader file is loaded, create the shader and constant buffer.
		auto createPSTask = loadPSTask.then([this](const std::vector<byte>& fileData) {
			ThrowIfFailed(
				mDeviceResources->GetD3DDevice()->CreatePixelShader(
					&fileData[0],
					fileData.size(),
					nullptr,
					mPixelShader.ReleaseAndGetAddressOf()
				)
			);

			CD3D11_BUFFER_DESC constantBufferDesc(sizeof(XMFLOAT4), D3D11_BIND_CONSTANT_BUFFER);
			ThrowIfFailed(
				mDeviceResources->GetD3DDevice()->CreateBuffer(
					&constantBufferDesc,
					nullptr,
					mPSCBufferPerObject.ReleaseAndGetAddressOf()
				)
			);
		});

		auto createVerticesAndBallsTask = (createPSTask && createVSTask).then([this]() {
			InitializeTriangleVertices();
			InitializeBar();
		});

		// Once the cube is loaded, the object is ready to be rendered.
		createVerticesAndBallsTask.then([this]() {
			mLoadingComplete = true;
		});
	}

	void BarManager::ReleaseDeviceDependentResources()
	{
		mLoadingComplete = false;
		mVertexShader.Reset();
		mPixelShader.Reset();
		mInputLayout.Reset();
		mTriangleVertexBuffer.Reset();
		mVSCBufferPerObject.Reset();
		mPSCBufferPerObject.Reset();
	}

	void BarManager::Update(const StepTimer& timer)
	{
		mBar->Update(timer);
	}

	void BarManager::Render(const StepTimer & timer)
	{
		UNREFERENCED_PARAMETER(timer);

		// Loading is asynchronous. Only draw geometry after it's loaded.
		if (!mLoadingComplete)
		{
			return;
		}

		ID3D11DeviceContext* direct3DDeviceContext = mDeviceResources->GetD3DDeviceContext();
		direct3DDeviceContext->IASetInputLayout(mInputLayout.Get());

		direct3DDeviceContext->VSSetShader(mVertexShader.Get(), nullptr, 0);
		direct3DDeviceContext->PSSetShader(mPixelShader.Get(), nullptr, 0);

		direct3DDeviceContext->VSSetConstantBuffers(0, 1, mVSCBufferPerObject.GetAddressOf());
		direct3DDeviceContext->PSSetConstantBuffers(0, 1, mPSCBufferPerObject.GetAddressOf());

		DrawBar(*mBar);
	}

	void BarManager::MoveRight()
	{
		DirectX::XMFLOAT2 currentVelocity = mBar->Velocity();
		
		if (currentVelocity.x < 0)
		{
			currentVelocity.x *= -1;
			mBar->SetVelocity(currentVelocity);
		}
	}

	void BarManager::MoveLeft()
	{
		DirectX::XMFLOAT2 currentVelocity = mBar->Velocity();

		if (currentVelocity.x > 0)
		{
			currentVelocity.x *= -1;
			mBar->SetVelocity(currentVelocity);
		}
	}

	float BarManager::HandleBallCollision(const DirectX::XMFLOAT2& ballPosition, const float& ballRadius, float& ballXVelocity)
	{
		float hitPosition = 0.0f;

		if ((ballPosition.y - ballRadius + 57) <= mBar->Position().y)
		{
			if (mBar->Position().x <= (ballPosition.x - ballRadius) && (ballPosition.x + ballRadius) <= (mBar->Position().x + (mBar->Width() * 2)))
			{
				hitPosition = mBar->Position().y - 54;

				if (ballPosition.x <= (mBar->Position().x + mBar->Width()) && ballXVelocity > 0)
				{
					ballXVelocity *= -1;
				}
				else if ((mBar->Position().x + mBar->Width()) < ballPosition.x && ballXVelocity < 0)
				{
					ballXVelocity *= -1;
				}
			}
		}

		return hitPosition;
	}

	bool BarManager::HandlePowerupCollision(const DirectX::XMFLOAT2& powerupPosition, const float& powerupWidth)
	{
		float powerupCenterX = powerupPosition.x + (powerupWidth / 2);

		if (mBar->Position().x <= powerupCenterX && powerupCenterX <= (mBar->Position().x + mBarWidth))
		{
			return true;
		}

		return false;
	}

	const std::int32_t BarManager::BarUpperY() const
	{
		return mBarY;
	}

	const std::int32_t BarManager::BarLowerY() const
	{
		return (mBarY - mBarHeight);
	}

	void BarManager::IncreaseBarVelocity()
	{
		mBar->SetVelocity(XMFLOAT2((mBar->Velocity().x + 30), mBar->Velocity().y));
	}

	void BarManager::DecreaseBarVelocity()
	{
		mBar->SetVelocity(XMFLOAT2((mBar->Velocity().x - 5), mBar->Velocity().y));
	}

	void BarManager::DrawBar(const Bar & bar)
	{
		ID3D11DeviceContext* direct3DDeviceContext = mDeviceResources->GetD3DDeviceContext();
		direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		static const UINT stride = sizeof(VertexPosition);
		static const UINT offset = 0;
		direct3DDeviceContext->IASetVertexBuffers(0, 1, mTriangleVertexBuffer.GetAddressOf(), &stride, &offset);

		const XMMATRIX wvp = XMMatrixTranspose(XMMatrixScaling(bar.Radius(), bar.Radius(), bar.Radius()) * bar.Transform().WorldMatrix() * mCamera->ViewProjectionMatrix());
		direct3DDeviceContext->UpdateSubresource(mVSCBufferPerObject.Get(), 0, nullptr, reinterpret_cast<const float*>(wvp.r), 0, 0);

		direct3DDeviceContext->UpdateSubresource(mPSCBufferPerObject.Get(), 0, nullptr, &bar.Color(), 0, 0);

		direct3DDeviceContext->Draw(SolidCircleVertexCount, 0);
	}

	void BarManager::InitializeTriangleVertices()
	{
		vector<VertexPosition> vertices;
		vertices.reserve(4);

		//Top left vertex
		VertexPosition topLeft;
		topLeft.Position.x = 0;
		topLeft.Position.y = -38;
		topLeft.Position.z = 0.0f;
		topLeft.Position.w = 1.0f;

		//Top right vertex
		VertexPosition topRight;
		topRight.Position.x = 8;
		topRight.Position.y = -38;
		topRight.Position.z = 0.0f;
		topRight.Position.w = 1.0f;

		//Bottom right vertex
		VertexPosition bottomLeft;
		bottomLeft.Position.x = 8;
		bottomLeft.Position.y = -40;
		bottomLeft.Position.z = 0.0f;
		bottomLeft.Position.w = 1.0f;

		//Bottom left vertex
		VertexPosition bottomRight;
		bottomRight.Position.x = 0;
		bottomRight.Position.y = -40;
		bottomRight.Position.z = 0.0f;
		bottomRight.Position.w = 1.0f;

		vertices.push_back(topLeft);
		vertices.push_back(topRight);
		vertices.push_back(bottomRight);
		vertices.push_back(bottomLeft);

		assert(vertices.size() == 4);

		D3D11_BUFFER_DESC vertexBufferDesc = { 0 };
		vertexBufferDesc.ByteWidth = sizeof(VertexPosition) * static_cast<uint32_t>(vertices.size());
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA vertexSubResourceData = { 0 };
		vertexSubResourceData.pSysMem = &vertices[0];
		ThrowIfFailed(mDeviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexSubResourceData, mTriangleVertexBuffer.ReleaseAndGetAddressOf()));
	}

	void BarManager::InitializeBar()
	{
		const float rotation = 0.0f;
		const float radius = 1.5f;
		const XMFLOAT4 color(&Colors::CornflowerBlue[0]);
		const XMFLOAT2 velocity(20, 0);
		const XMFLOAT2 position(-(float)(mBarWidth / 2 + 2), 15);

		mBar = make_shared<Bar>(*this, Transform2D(position), radius, color, velocity);
	}
}