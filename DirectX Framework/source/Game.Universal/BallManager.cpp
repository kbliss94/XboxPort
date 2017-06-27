#include "pch.h"
#include "BallManager.h"
#include "Ball.h"

using namespace std;
using namespace DirectX;
using namespace DX;

namespace DirectXGame
{
	const uint32_t BallManager::CircleResolution = 32;
	const uint32_t BallManager::LineCircleVertexCount = BallManager::CircleResolution + 2;
	const uint32_t BallManager::SolidCircleVertexCount = (BallManager::CircleResolution + 1) * 2;

	BallManager::BallManager(const shared_ptr<DX::DeviceResources>& deviceResources, const shared_ptr<Camera>& camera, ChunkManager& chunkManager, BarManager& barManager) :
		DrawableGameComponent(deviceResources, camera),
		mLoadingComplete(false), mChunkManager(chunkManager), mBarManager(barManager), mBallLaunched(false)
	{
		CreateDeviceDependentResources();
	}

	std::shared_ptr<Field> BallManager::ActiveField() const
	{
		return mActiveField;
	}

	void BallManager::SetActiveField(const shared_ptr<Field>& field)
	{
		mActiveField = field;
	}

	void BallManager::CreateDeviceDependentResources()
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
			InitializeLineVertices();
			InitializeTriangleVertices();
			InitializeBall();
		});

		// Once the cube is loaded, the object is ready to be rendered.
		createVerticesAndBallsTask.then([this]() {
			mLoadingComplete = true;
		});
	}

	void BallManager::ReleaseDeviceDependentResources()
	{
		mLoadingComplete = false;
		mVertexShader.Reset();
		mPixelShader.Reset();
		mInputLayout.Reset();
		mLineVertexBuffer.Reset();
		mTriangleVertexBuffer.Reset();
		mVSCBufferPerObject.Reset();
		mPSCBufferPerObject.Reset();
	}

	void BallManager::Update(const StepTimer& timer)
	{
		mBall->Update(timer);
	}

	void BallManager::Render(const StepTimer & timer)
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

		DrawSolidBall(*mBall);
	}

	void BallManager::IncreaseBallVelocity()
	{
		XMFLOAT2 newVelocity = mBall->Velocity();

		if (mBall->Velocity().x < 0)
		{
			newVelocity.x -= 5;
		}
		else if (mBall->Velocity().x > 0)
		{
			newVelocity.x += 5;
		}

		if (mBall->Velocity().y < 0)
		{
			newVelocity.y -= 5;
		}
		else if (mBall->Velocity().y > 0)
		{
			newVelocity.y += 5;
		}

		mBall->SetVelocity(newVelocity);
	}

	void BallManager::DecreaseBallVelocity()
	{
		XMFLOAT2 newVelocity = mBall->Velocity();

		if (mBall->Velocity().x < 0)
		{
			newVelocity.x += 5;
		}
		else if (mBall->Velocity().x > 0)
		{
			newVelocity.x -= 5;
		}

		if (mBall->Velocity().y < 0)
		{
			newVelocity.y += 5;
		}
		else if (mBall->Velocity().y > 0)
		{
			newVelocity.y -= 5;
		}

		mBall->SetVelocity(newVelocity);
	}

	void BallManager::DrawSolidBall(const Ball & ball)
	{
		ID3D11DeviceContext* direct3DDeviceContext = mDeviceResources->GetD3DDeviceContext();
		direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		static const UINT stride = sizeof(VertexPosition);
		static const UINT offset = 0;
		direct3DDeviceContext->IASetVertexBuffers(0, 1, mTriangleVertexBuffer.GetAddressOf(), &stride, &offset);

		const XMMATRIX wvp = XMMatrixTranspose(XMMatrixScaling(ball.Radius(), ball.Radius(), ball.Radius()) * ball.Transform().WorldMatrix() * mCamera->ViewProjectionMatrix());
		direct3DDeviceContext->UpdateSubresource(mVSCBufferPerObject.Get(), 0, nullptr, reinterpret_cast<const float*>(wvp.r), 0, 0);

		direct3DDeviceContext->UpdateSubresource(mPSCBufferPerObject.Get(), 0, nullptr, &ball.Color(), 0, 0);

		direct3DDeviceContext->Draw(SolidCircleVertexCount, 0);
	}

	const bool BallManager::LaunchedBall() const
	{
		return mBallLaunched;
	}

	void BallManager::LaunchBall()
	{
		mBall->SetVelocity(mInitialVelocity);
		mBallLaunched = true;
	}

	void BallManager::BallOffscreen()
	{
		mChunkManager.GameOver();
	}

	void BallManager::InitializeLineVertices()
	{
		const float increment = XM_2PI / CircleResolution;

		VertexPosition vertices[LineCircleVertexCount];

		for (int i = 0; i < CircleResolution; i++)
		{
			VertexPosition& vertex = vertices[i];

			vertex.Position.x = cosf(i * increment);
			vertex.Position.y = sinf(i * increment);
			vertex.Position.z = 0.0f;
			vertex.Position.w = 1.0f;
		}

		// Closing line to complete the circle
		vertices[CircleResolution] = VertexPosition(vertices[0]);

		// Axis line for visualizing rotation
		vertices[CircleResolution + 1] = VertexPosition(XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));

		D3D11_BUFFER_DESC vertexBufferDesc = { 0 };
		vertexBufferDesc.ByteWidth = sizeof(VertexPosition) * LineCircleVertexCount;
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA vertexSubResourceData = { 0 };
		vertexSubResourceData.pSysMem = vertices;
		ThrowIfFailed(mDeviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexSubResourceData, mLineVertexBuffer.ReleaseAndGetAddressOf()));
	}

	void BallManager::InitializeTriangleVertices()
	{
		const float increment = XM_2PI / CircleResolution;
		const XMFLOAT4 center(0.0f, 0.0f, 0.0f, 1.0f);

		vector<VertexPosition> vertices;
		vertices.reserve(SolidCircleVertexCount);
		for (int i = 0; i <= CircleResolution; i++)
		{
			VertexPosition vertex;
			vertex.Position.x = cosf(i * increment);
			vertex.Position.y = sinf(i * increment);
			vertex.Position.z = 0.0f;
			vertex.Position.w = 1.0f;

			vertices.push_back(vertex);
			vertices.push_back(center);
		}

		assert(vertices.size() == SolidCircleVertexCount);

		D3D11_BUFFER_DESC vertexBufferDesc = { 0 };
		vertexBufferDesc.ByteWidth = sizeof(VertexPosition) * static_cast<uint32_t>(vertices.size());
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA vertexSubResourceData = { 0 };
		vertexSubResourceData.pSysMem = &vertices[0];
		ThrowIfFailed(mDeviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexSubResourceData, mTriangleVertexBuffer.ReleaseAndGetAddressOf()));
	}

	void BallManager::InitializeBall()
	{
		const float rotation = 0.5f;
		const float radius = 1.5f;
		const XMFLOAT4 color(&Colors::PeachPuff[0]);
		const XMFLOAT2 velocity(0, 0);

		mBall = make_shared<Ball>(*this, mChunkManager, mBarManager, Transform2D(XMFLOAT2(0, (-3*(float)(mBarManager.BarUpperY()) - (radius*5))), rotation), radius,
			color, velocity);
	}
}
