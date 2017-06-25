#include "pch.h"
#include "ChunkManager.h"
#include "Chunk.h"

using namespace std;
using namespace DirectX;
using namespace DX;

namespace DirectXGame
{
	const uint32_t ChunkManager::CircleResolution = 32;
	const uint32_t ChunkManager::SolidCircleVertexCount = (ChunkManager::CircleResolution + 1) * 2;

	ChunkManager::ChunkManager(const shared_ptr<DX::DeviceResources>& deviceResources, const shared_ptr<Camera>& camera) :
		DrawableGameComponent(deviceResources, camera), mLoadingComplete(false)
	{
		CreateDeviceDependentResources();
	}

	std::shared_ptr<Field> ChunkManager::ActiveField() const
	{
		return mActiveField;
	}

	void ChunkManager::SetActiveField(const shared_ptr<Field>& field)
	{
		mActiveField = field;
	}

	void ChunkManager::CreateDeviceDependentResources()
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
			InitializeChunks();
		});

		// Once the cube is loaded, the object is ready to be rendered.
		createVerticesAndBallsTask.then([this]() {
			mLoadingComplete = true;
		});
	}

	void ChunkManager::ReleaseDeviceDependentResources()
	{
		mLoadingComplete = false;
		mVertexShader.Reset();
		mPixelShader.Reset();
		mInputLayout.Reset();
		mTriangleVertexBuffer.Reset();
		mVSCBufferPerObject.Reset();
		mPSCBufferPerObject.Reset();
	}

	void ChunkManager::Update(const StepTimer& timer)
	{
		for (const auto& chunk : mChunks)
		{
			chunk->Update(timer);
		}
	}

	void ChunkManager::Render(const StepTimer & timer)
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

		for (const auto& chunk : mChunks)
		{
			DrawChunk(*chunk);
		}
	}

	void ChunkManager::DrawChunk(const Chunk & chunk)
	{
		ID3D11DeviceContext* direct3DDeviceContext = mDeviceResources->GetD3DDeviceContext();
		direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		static const UINT stride = sizeof(VertexPosition);
		static const UINT offset = 0;
		direct3DDeviceContext->IASetVertexBuffers(0, 1, mTriangleVertexBuffer.GetAddressOf(), &stride, &offset);

		const XMMATRIX wvp = XMMatrixTranspose(XMMatrixScaling(chunk.Radius(), chunk.Radius(), chunk.Radius()) * chunk.Transform().WorldMatrix() * mCamera->ViewProjectionMatrix());
		direct3DDeviceContext->UpdateSubresource(mVSCBufferPerObject.Get(), 0, nullptr, reinterpret_cast<const float*>(wvp.r), 0, 0);

		direct3DDeviceContext->UpdateSubresource(mPSCBufferPerObject.Get(), 0, nullptr, &chunk.Color(), 0, 0);

		direct3DDeviceContext->Draw(SolidCircleVertexCount, 0);
	}

	float ChunkManager::HandleBallCollision(const XMFLOAT2& ballPosition, const float& ballRadius)
	{
		//checking to see if this position collides w/a chunk position
			//aka >= chunk bottom (chunk position.y - chunk height)
		float hitPosition = 0.0f;

		for (auto it = mChunks.begin(); it != mChunks.end(); ++it)
		{
			if ((ballPosition.y + ballRadius + 57) >= ((*it)->Position().y - mChunkHeight))
			{
				if (((*it)->Position().x) <= ballPosition.x && ballPosition.x <= ((*it)->Position().x + mChunkWidth))
				{
					hitPosition = ((*it)->Position().y - mChunkHeight) - 58;
					mChunks.erase(it);
					break;
				}
			}
		}

		return hitPosition;
	}

	void ChunkManager::InitializeTriangleVertices()
	{
		vector<VertexPosition> vertices;
		vertices.reserve(4);

		//top left vertex
		VertexPosition topLeft;
		topLeft.Position.x = 0;
		topLeft.Position.y = -38;
		topLeft.Position.z = 0.0f;
		topLeft.Position.w = 1.0f;

		//top right vertex
		VertexPosition topRight;
		topRight.Position.x = 6;
		topRight.Position.y = -38;
		topRight.Position.z = 0.0f;
		topRight.Position.w = 1.0f;

		//bottom right vertex
		VertexPosition bottomLeft;
		bottomLeft.Position.x = 6;
		bottomLeft.Position.y = -40;
		bottomLeft.Position.z = 0.0f;
		bottomLeft.Position.w = 1.0f;

		//bottom left vertex
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

	void ChunkManager::InitializeChunks()
	{
		const float rotation = 0.0f;
		const float radius = 1.5f;
		const XMFLOAT2 velocity(0, 0);
		float originalX = -45.0f;
		XMFLOAT2 position(originalX, 97);
		int colorIndex = 0;

		if (mChunks.size() < mNumChunks)
		{
			for (uint32_t i = 1; i <= mNumChunks; ++i)
			{
				//mChunks.emplace_back(make_shared<Chunk>(*this, position, radius, mChunkColors[colorIndex], velocity));
				mChunks.emplace(mChunks.begin(), make_shared<Chunk>(*this, position, radius, mChunkColors[colorIndex], velocity));


				//if (i % 15 == 0)
				if (i % 10 == 0)
				{
					colorIndex += 1;

					//moving downwards to the next row
					//position = XMFLOAT2(originalX - 6, (position.y - 3));
					position = XMFLOAT2(originalX - mChunkWidth, (position.y - mChunkHeight));
				}

				//moving to the right to the next column
				position = XMFLOAT2((position.x + mChunkWidth), position.y);
			}
		}
	}
}