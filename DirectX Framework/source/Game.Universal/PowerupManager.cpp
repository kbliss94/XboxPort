#include "pch.h"
#include "ChunkManager.h"
#include "Chunk.h"

using namespace std;
using namespace DirectX;
using namespace DX;

namespace DirectXGame
{
	const uint32_t PowerupManager::CircleResolution = 32;
	const uint32_t PowerupManager::SolidCircleVertexCount = (PowerupManager::CircleResolution + 1) * 2;

	PowerupManager::PowerupManager(const shared_ptr<DX::DeviceResources>& deviceResources, const shared_ptr<Camera>& camera,
		BarManager& barManager) :
		DrawableGameComponent(deviceResources, camera), mLoadingComplete(false), 
		mBarManager(barManager)
	{
		CreateDeviceDependentResources();
	}

	std::shared_ptr<Field> PowerupManager::ActiveField() const
	{
	return mActiveField;
	}

	void PowerupManager::SetActiveField(const shared_ptr<Field>& field)
	{
		mActiveField = field;
	}

	void PowerupManager::CreateDeviceDependentResources()
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
			//InitializePowerup();
		});

		// Once the cube is loaded, the object is ready to be rendered.
		createVerticesAndBallsTask.then([this]() {
			mLoadingComplete = true;
		});
	}

	void PowerupManager::ReleaseDeviceDependentResources()
	{
		mLoadingComplete = false;
		mVertexShader.Reset();
		mPixelShader.Reset();
		mInputLayout.Reset();
		mTriangleVertexBuffer.Reset();
		mVSCBufferPerObject.Reset();
		mPSCBufferPerObject.Reset();
	}

	void PowerupManager::Update(const StepTimer& timer)
	{
		for (const auto& powerup : mPowerups)
		{
			powerup->Update(timer);
		}

		for (auto it = mPowerups.begin(); it != mPowerups.end(); ++it)
		{
			//if (powerup y + powerup height) is within bar y, check for collision
			if (((*it)->Position().y + mPowerupHeight) <= mBarManager.BarUpperY())
			{
				if (mBarManager.HandlePowerupCollision((*it)->Position(), mPowerupWidth))
				{
					if (!(*it)->Activated())
					{
						(*it)->ActivatePowerup();

						//trigger the appropriate powerup effect
						if ((*it)->Type() == Powerup::FasterBar)
						{
							mBarManager.IncreaseBarVelocity();
						}
						else if ((*it)->Type() == Powerup::SlowerBar)
						{
							mBarManager.DecreaseBarVelocity();
						}
						else if ((*it)->Type() == Powerup::FasterBall)
						{
							mBallManager->IncreaseBallVelocity();
						}
						else if ((*it)->Type() == Powerup::SlowerBall)
						{
							mBallManager->DecreaseBallVelocity();
						}
					}

					//trigger the appropriate powerup effect
					//if (mPowerups.size() != 1)
					//{
					//	mPowerups.erase(it);
					//}
				}
			}
		}

		for (auto it = mPowerups.begin(); it != mPowerups.end(); ++it)
		{
			//if powerup y is within bottom of screen(/bar), delete the chunk 
			//if ((*it)->Position().y <= (mBarManager.BarLowerY()))
			if ((*it)->Position().y <= 0)
			{
				(*it)->ActivatePowerup();
				//if (mPowerups.size() != 1)
				//{
				//	mPowerups.erase(it);
				//}
			}
		}

		//for (const auto& powerup : mPowerups)
		//{
		//	if (!(powerup->Activated()))
		//	{
		//		powerup->Update(timer);
		//	}
		//}
	}

	void PowerupManager::Render(const StepTimer & timer)
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

		for (const auto& powerup : mPowerups)
		{
			DrawPowerup(*powerup);
		}
	}

	void PowerupManager::DrawPowerup(const Powerup & powerup)
	{
		ID3D11DeviceContext* direct3DDeviceContext = mDeviceResources->GetD3DDeviceContext();
		direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		static const UINT stride = sizeof(VertexPosition);
		static const UINT offset = 0;
		direct3DDeviceContext->IASetVertexBuffers(0, 1, mTriangleVertexBuffer.GetAddressOf(), &stride, &offset);

		const XMMATRIX wvp = XMMatrixTranspose(XMMatrixScaling(powerup.Radius(), powerup.Radius(), powerup.Radius()) * powerup.Transform().WorldMatrix() * mCamera->ViewProjectionMatrix());
		direct3DDeviceContext->UpdateSubresource(mVSCBufferPerObject.Get(), 0, nullptr, reinterpret_cast<const float*>(wvp.r), 0, 0);

		direct3DDeviceContext->UpdateSubresource(mPSCBufferPerObject.Get(), 0, nullptr, &powerup.Color(), 0, 0);

		direct3DDeviceContext->Draw(SolidCircleVertexCount, 0);
	}

	//float PowerupManager::HandleBarCollision(const XMFLOAT2& ballPosition, const float& ballRadius)
	//{
	//	UNREFERENCED_PARAMETER(ballPosition);
	//	UNREFERENCED_PARAMETER(ballRadius);

	//	//should prob have a public enum in Powerup so you can check which powerup was caught

	//	return 0.0f;
	//	//float hitPosition = 0.0f;

	//	//for (auto it = mPowerups.begin(); it != mPowerups.end(); ++it)
	//	//{
	//	//	if ((ballPosition.y + ballRadius + 57) >= ((*it)->Position().y - mChunkHeight))
	//	//	{
	//	//		if (((*it)->Position().x) <= ballPosition.x && ballPosition.x <= ((*it)->Position().x + mChunkWidth))
	//	//		{
	//	//			hitPosition = ((*it)->Position().y - mChunkHeight) - 58;
	//	//			mPowerups.erase(it);
	//	//			break;
	//	//		}
	//	//	}
	//	//}

	//	//return hitPosition;
	//}

	void PowerupManager::SetBallManager(std::shared_ptr<BallManager> ballManager)
	{
		mBallManager = ballManager;
	}

	void PowerupManager::PowerupSpawnCheck(const XMFLOAT2& chunkPosition)
	{
		//probability check to see if a powerup should be spawned (.25 chance)
		//random_device device;
		//default_random_engine generator(device());
		//uniform_int_distribution<uint32_t> spawnDistribution(0, 3);

		//if (spawnDistribution(generator) == 0)
		//{
		//	SpawnPowerup(chunkPosition);
		//}

		SpawnPowerup(chunkPosition);
	}

	void PowerupManager::SpawnPowerup(const XMFLOAT2& chunkPosition)
	{
		//randomly selecting powerup effect (& associated color) here
		random_device device;
		default_random_engine generator(device());
		uniform_int_distribution<uint32_t> powerupDistribution(0, 3);

		//uint32_t powerupSelection = powerupDistribution(generator);
		uint32_t powerupSelection = 0;

		InitializePowerup(chunkPosition, mPossiblePowerups[powerupSelection].Type, mPossiblePowerups[powerupSelection].Color);
	}

	void PowerupManager::InitializeTriangleVertices()
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
		topRight.Position.x = 3;
		topRight.Position.y = -38;
		topRight.Position.z = 0.0f;
		topRight.Position.w = 1.0f;

		//bottom right vertex
		VertexPosition bottomLeft;
		bottomLeft.Position.x = 3;
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

	void PowerupManager::InitializePowerup(const XMFLOAT2& position, Powerup::PowerupType type, const DirectX::XMFLOAT4& color)
	{
		const float rotation = 0.0f;
		const float radius = 1.5f;
		const XMFLOAT2 velocity(0, -10);
		//int colorIndex = 0;
		
		//mPowerups.emplace_back(make_shared<Powerup>(*this, position, radius, mChunkColors[colorIndex], velocity));
		mPowerups.emplace_back(make_shared<Powerup>(*this, position, radius, color, velocity, type));
	}
}