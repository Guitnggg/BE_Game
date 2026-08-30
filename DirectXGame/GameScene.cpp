#include "GameScene.h"

#include <KamataEngine.h>
#include <algorithm>
#include <cmath>
#include <dinput.h>
#include <functional>
#include <string>

namespace {
// API仕様が決まるまで使用するサーバー代替領域。
// SubmitScore()だけをHTTP送受信に置き換えればゲーム本体は変更不要。
std::vector<int> gMockServerScores = {975, 910, 825, 700, 550};
constexpr float kTargetSeconds = 10.0f;
constexpr float kHideTimerSeconds = 7.0f;
constexpr int kPerfectScore = 1000;
constexpr std::size_t kRankingCount = 5;
} // namespace

void GameScene::Initialize() {
	KamataEngine::DebugText::GetInstance()->Initialize();
	SubmitScore();
	phase_ = Phase::Ready;
}

void GameScene::Update() {
	auto* const input = KamataEngine::Input::GetInstance();
	if (!input->TriggerKey(DIK_SPACE)) {
		return;
	}
	switch (phase_) {
	case Phase::Ready:
	case Phase::Ranking:
		StartGame();
		break;
	case Phase::Playing:
		StopGame();
		break;
	}
}

void GameScene::Draw() {
	auto* const debugText = KamataEngine::DebugText::GetInstance();
	KamataEngine::Sprite::PreDraw();
	debugText->Print("10 SECOND STOP CHALLENGE", 380.0f, 80.0f, 2.0f);

	switch (phase_) {
	case Phase::Ready:
		debugText->Print("Press SPACE to start the timer", 420.0f, 260.0f, 1.5f);
		debugText->Print("Stop as close to 10.000 seconds as possible!", 360.0f, 320.0f, 1.2f);
		debugText->Print("The timer disappears after 7 seconds.", 400.0f, 360.0f, 1.0f);
		break;
	case Phase::Playing: {
		const float elapsed = GetElapsedSeconds();
		debugText->Print("Press SPACE to stop", 470.0f, 250.0f, 1.5f);
		if (elapsed < kHideTimerSeconds) {
			debugText->Print("TIME", 565.0f, 330.0f, 1.2f);
			debugText->Print(std::to_string(elapsed).substr(0, 5), 535.0f, 370.0f, 2.0f);
		} else {
			debugText->Print("TIME: ???", 530.0f, 350.0f, 1.5f);
			debugText->Print("Trust your sense of time...", 445.0f, 410.0f, 1.0f);
		}
		break;
	}
	case Phase::Ranking:
		debugText->Print("RESULT", 545.0f, 175.0f, 1.5f);
		debugText->Print("STOP TIME: " + std::to_string(stoppedTime_).substr(0, 6) + " sec", 450.0f, 225.0f, 1.2f);
		debugText->Print("SCORE: " + std::to_string(score_), 500.0f, 270.0f, 1.5f);
		if (score_ == 0 && stoppedTime_ > kTargetSeconds) {
			debugText->Print("BUST! You went over 10 seconds.", 430.0f, 315.0f, 1.0f);
		}
		debugText->Print("SERVER TOP 5", 510.0f, 370.0f, 1.2f);
		for (std::size_t i = 0; i < ranking_.size(); ++i) {
			debugText->Print(std::to_string(i + 1) + ".  " + std::to_string(ranking_[i]) + " pts", 535.0f, 405.0f + static_cast<float>(i) * 28.0f, 1.0f);
		}
		debugText->Print("Press SPACE to retry", 500.0f, 585.0f, 1.2f);
		break;
	}
	debugText->DrawAll();
	KamataEngine::Sprite::PostDraw();
}

void GameScene::StartGame() {
	startTime_ = Clock::now();
	stoppedTime_ = 0.0f;
	score_ = 0;
	phase_ = Phase::Playing;
}

void GameScene::StopGame() {
	stoppedTime_ = GetElapsedSeconds();
	if (stoppedTime_ > kTargetSeconds) {
		score_ = 0;
	} else {
		const int errorMilliseconds = static_cast<int>(std::lround((kTargetSeconds - stoppedTime_) * 1000.0f));
		score_ = (std::max)(0, kPerfectScore - errorMilliseconds);
	}
	SubmitScore();
	phase_ = Phase::Ranking;
}

void GameScene::SubmitScore() {
	if (phase_ == Phase::Playing) {
		gMockServerScores.push_back(score_);
	}
	std::ranges::sort(gMockServerScores, std::greater<>());
	if (gMockServerScores.size() > kRankingCount) {
		gMockServerScores.resize(kRankingCount);
	}
	ranking_ = gMockServerScores;
}

float GameScene::GetElapsedSeconds() const {
	return std::chrono::duration<float>(Clock::now() - startTime_).count();
}
