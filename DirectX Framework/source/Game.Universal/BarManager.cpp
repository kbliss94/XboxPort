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
		DrawableGameComponent(deviceResources, camera),
		mLoadingComplete(false)
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

	float BarManager::HandleBallCollision(const DirectX::XMFLOAT2& ballPosition, const float& ballRadius)
	{
		UNREFERENCED_PARAMETER(ballPosition);
		UNREFERENCED_PARAMETER(ballRadius);

		return 0.0f;
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

		//top left vertex
		VertexPosition topLeft;
		topLeft.Position.x = 0;
		topLeft.Position.y = -38;
		topLeft.Position.z = 0.0f;
		topLeft.Position.w = 1.0f;

		//top right vertex
		VertexPosition topRight;
		topRight.Position.x = 8;
		topRight.Position.y = -38;
		topRight.Position.z = 0.0f;
		topRight.Position.w = 1.0f;

		//bottom right vertex
		VertexPosition bottomLeft;
		bottomLeft.Position.x = 8;
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

	void BarManager::InitializeBar()
	{
		const float rotation = 0.0f;
		const float radius = 1.5f;
		const XMFLOAT4 color(&Colors::CornflowerBlue[0]);
		const XMFLOAT2 velocity(20, 0);

		mBar = make_shared<Bar>(*this, Transform2D(DirectX::XMFLOAT2(0, 15)), radius, color, velocity);
	}
}






//#include "pch.h"
//#include "BarManager.h"
//#include "Bar.h"
//
//using namespace std;
//using namespace DirectX;
//using namespace SimpleMath;
//using namespace DX;
//
//namespace DirectXGame
//{
//	const uint32_t BarManager::SolidCircleVertexCount = 4;
//
//	BarManager::BarManager(const shared_ptr<DX::DeviceResources>& deviceResources, const shared_ptr<Camera>& camera) :
//		DrawableGameComponent(deviceResources, camera), mLoadingComplete(false), mIndexCount(0)
//	{
//		CreateDeviceDependentResources();
//	}
//
//	shared_ptr<Field> BarManager::ActiveField() const
//	{
//		return mActiveField;
//	}
//
//	void BarManager::SetActiveField(const std::shared_ptr<Field>& field)
//	{
//		mActiveField = field;
//	}
//
//	void BarManager::CreateDeviceDependentResources()
//	{
//		// Create a field
//		const XMFLOAT2 position = Vector2Helper::Zero;
//		const XMFLOAT2 size(95, 80);
//		const XMFLOAT4 color(&Colors::AntiqueWhite[0]);
//
//		//Create the bar
//		const XMFLOAT2 barSize(10, 10);
//		const XMFLOAT4 barColor(&Colors::HotPink[0]);
//		const XMFLOAT2 barPosition(0, -40);
//		const XMFLOAT2 barVelocity(20, 0);
//
//		mActiveField = make_shared<Field>(position, size, color);
//		mBar = make_shared<Bar>(barPosition, barSize, barColor, barVelocity, *this);
//		//mBar = make_shared<Bar>(Transform2D(Vector2Helper::Zero), barColor, barVelocity, *this);
//
//		auto loadVSTask = ReadDataAsync(L"ShapeRendererVS.cso");
//		auto loadPSTask = ReadDataAsync(L"ShapeRendererPS.cso");
//
//		// After the vertex shader file is loaded, create the shader and input layout.
//		auto createVSTask = loadVSTask.then([this](const vector<byte>& fileData) {
//			ThrowIfFailed(
//				mDeviceResources->GetD3DDevice()->CreateVertexShader(
//					&fileData[0],
//					fileData.size(),
//					nullptr,
//					mVertexShader.ReleaseAndGetAddressOf()
//				)
//			);
//
//			// Create an input layout
//			ThrowIfFailed(
//				mDeviceResources->GetD3DDevice()->CreateInputLayout(
//					VertexPosition::InputElements,
//					VertexPosition::InputElementCount,
//					&fileData[0],
//					fileData.size(),
//					mInputLayout.ReleaseAndGetAddressOf()
//				)
//			);
//
//			CD3D11_BUFFER_DESC constantBufferDesc(sizeof(XMFLOAT4X4), D3D11_BIND_CONSTANT_BUFFER);
//			ThrowIfFailed(
//				mDeviceResources->GetD3DDevice()->CreateBuffer(
//					&constantBufferDesc,
//					nullptr,
//					mVSCBufferPerObject.ReleaseAndGetAddressOf()
//				)
//			);
//		});
//
//		// After the pixel shader file is loaded, create the shader and constant buffer.
//		auto createPSTask = loadPSTask.then([this](const vector<byte>& fileData) {
//			ThrowIfFailed(
//				mDeviceResources->GetD3DDevice()->CreatePixelShader(
//					&fileData[0],
//					fileData.size(),
//					nullptr,
//					mPixelShader.ReleaseAndGetAddressOf()
//				)
//			);
//
//			CD3D11_BUFFER_DESC constantBufferDesc(sizeof(XMFLOAT4), D3D11_BIND_CONSTANT_BUFFER);
//			ThrowIfFailed(
//				mDeviceResources->GetD3DDevice()->CreateBuffer(
//					&constantBufferDesc,
//					nullptr,
//					mPSCBufferPerObject.ReleaseAndGetAddressOf()
//				)
//			);
//		});
//
//		(createPSTask && createVSTask).then([this]() {
//			// Create a vertex buffer for rendering a box
//			D3D11_BUFFER_DESC vertexBufferDesc = { 0 };
//			const uint32_t boxVertexCount = 4;
//			vertexBufferDesc.ByteWidth = sizeof(VertexPosition) * boxVertexCount;
//			vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
//			vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
//			vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
//			ThrowIfFailed(mDeviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, nullptr, mVertexBuffer.ReleaseAndGetAddressOf()));
//
//			// Create an index buffer for the box (line strip)
//			uint32_t indices[] =
//			{
//				0, 1, 2, 3, 0
//			};
//
//			mIndexCount = ARRAYSIZE(indices);
//
//			D3D11_BUFFER_DESC indexBufferDesc = { 0 };
//			indexBufferDesc.ByteWidth = sizeof(uint32_t) * mIndexCount;
//			indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
//			indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
//
//			D3D11_SUBRESOURCE_DATA indexSubResourceData = { 0 };
//			indexSubResourceData.pSysMem = indices;
//			ThrowIfFailed(mDeviceResources->GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexSubResourceData, mIndexBuffer.ReleaseAndGetAddressOf()));
//			mLoadingComplete = true;
//		});
//
//		InitializeTriangleVertices();
//	}
//
//	void BarManager::ReleaseDeviceDependentResources()
//	{
//		mLoadingComplete = false;
//		mVertexShader.Reset();
//		mPixelShader.Reset();
//		mInputLayout.Reset();
//		mVertexBuffer.Reset();
//		mVertexBuffer.Reset();
//		mVSCBufferPerObject.Reset();
//		mPSCBufferPerObject.Reset();
//
//		mTriangleVertexBuffer.Reset();
//	}
//
//	void BarManager::Update(const DX::StepTimer& timer)
//	{
//		mBar->Update(timer);
//	}
//
//	void BarManager::Render(const StepTimer & timer)
//	{
//		UNREFERENCED_PARAMETER(timer);
//
//		// Loading is asynchronous. Only draw geometry after it's loaded.
//		if (!mLoadingComplete)
//		{
//			return;
//		}
//
//		ID3D11DeviceContext* direct3DDeviceContext = mDeviceResources->GetD3DDeviceContext();
//		direct3DDeviceContext->IASetInputLayout(mInputLayout.Get());
//
//		direct3DDeviceContext->VSSetShader(mVertexShader.Get(), nullptr, 0);
//		direct3DDeviceContext->PSSetShader(mPixelShader.Get(), nullptr, 0);
//
//		direct3DDeviceContext->VSSetConstantBuffers(0, 1, mVSCBufferPerObject.GetAddressOf());
//		direct3DDeviceContext->PSSetConstantBuffers(0, 1, mPSCBufferPerObject.GetAddressOf());
//
//		DrawBar(*mBar);
//	}
//
//	void BarManager::DrawBar(const Bar& bar)
//	{
//		ID3D11DeviceContext* direct3DDeviceContext = mDeviceResources->GetD3DDeviceContext();
//		direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
//
//		static const UINT stride = sizeof(VertexPosition);
//		static const UINT offset = 0;
//		direct3DDeviceContext->IASetVertexBuffers(0, 1, mTriangleVertexBuffer.GetAddressOf(), &stride, &offset);
//
//		const XMMATRIX wvp = XMMatrixTranspose(mCamera->ViewProjectionMatrix());
//		direct3DDeviceContext->UpdateSubresource(mVSCBufferPerObject.Get(), 0, nullptr, reinterpret_cast<const float*>(wvp.r), 0, 0);
//		direct3DDeviceContext->UpdateSubresource(mPSCBufferPerObject.Get(), 0, nullptr, &bar.Color(), 0, 0);
//
//		direct3DDeviceContext->Draw(SolidCircleVertexCount, 0);
//	}
//
//	void BarManager::MoveRight()
//	{
//		DirectX::XMFLOAT2 currentVelocity = mBar->Velocity();
//
//		if (currentVelocity.x < 0)
//		{
//			currentVelocity.x *= -1;
//			mBar->SetVelocity(currentVelocity);
//		}
//	}
//
//	void BarManager::MoveLeft()
//	{
//		DirectX::XMFLOAT2 currentVelocity = mBar->Velocity();
//
//		if (currentVelocity.x > 0)
//		{
//			currentVelocity.x *= -1;
//			mBar->SetVelocity(currentVelocity);
//		}
//	}
//
//	void BarManager::InitializeTriangleVertices()
//	{
//		vector<VertexPosition> vertices;
//		vertices.reserve(4);
//		
//		//top left vertex
//		VertexPosition topLeft;
//		topLeft.Position.x = 0;
//		topLeft.Position.y = -38;
//		topLeft.Position.z = 0.0f;
//		topLeft.Position.w = 1.0f;
//
//		//top right vertex
//		VertexPosition topRight;
//		topRight.Position.x = 10;
//		topRight.Position.y = -38;
//		topRight.Position.z = 0.0f;
//		topRight.Position.w = 1.0f;
//
//		//bottom right vertex
//		VertexPosition bottomLeft;
//		bottomLeft.Position.x = 10;
//		bottomLeft.Position.y = -40;	
//		bottomLeft.Position.z = 0.0f;
//		bottomLeft.Position.w = 1.0f;
//
//		//bottom left vertex
//		VertexPosition bottomRight;
//		bottomRight.Position.x = 0;
//		bottomRight.Position.y = -40;
//		bottomRight.Position.z = 0.0f;
//		bottomRight.Position.w = 1.0f;
//
//		vertices.push_back(topLeft);
//		vertices.push_back(topRight);
//		vertices.push_back(bottomRight);
//		vertices.push_back(bottomLeft);
//
//		assert(vertices.size() == SolidCircleVertexCount);
//
//		D3D11_BUFFER_DESC vertexBufferDesc = { 0 };
//		vertexBufferDesc.ByteWidth = sizeof(VertexPosition) * static_cast<uint32_t>(vertices.size());
//		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
//		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
//
//		D3D11_SUBRESOURCE_DATA vertexSubResourceData = { 0 };
//		vertexSubResourceData.pSysMem = &vertices[0];
//		ThrowIfFailed(mDeviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexSubResourceData, mTriangleVertexBuffer.ReleaseAndGetAddressOf()));
//	}
//}