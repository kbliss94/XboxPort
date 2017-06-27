#pragma once

#include "DrawableGameComponent.h"
#include "StepTimer.h"
#include <string>

namespace DirectXGame
{
	// Renders the score in the top center of the screen using Direct2D and DirectWrite.
	class ScoreManager final : public DX::DrawableGameComponent
	{
	public:
		ScoreManager(const std::shared_ptr<DX::DeviceResources>& deviceResources);

		virtual void CreateDeviceDependentResources() override;
		virtual void ReleaseDeviceDependentResources() override;
		virtual void Update(const DX::StepTimer& timer) override;
		virtual void Render(const DX::StepTimer& timer) override;

		void IncrementScore();
		void SetGameOver();
		const bool IsGameOver();
		void SetBallLaunched();

	private:
		std::wstring                                    m_text;
		DWRITE_TEXT_METRICS	                            m_textMetrics;
		Microsoft::WRL::ComPtr<ID2D1SolidColorBrush>    m_whiteBrush;
		Microsoft::WRL::ComPtr<ID2D1DrawingStateBlock1> m_stateBlock;
		Microsoft::WRL::ComPtr<IDWriteTextLayout3>      m_textLayout;
		Microsoft::WRL::ComPtr<IDWriteTextFormat2>      m_textFormat;

		std::int32_t mScore;
		bool mGameOver;
		bool mBallLaunched;
	};
}
