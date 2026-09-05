#include "GameScene.h"
#include "HttpClient.h"

#include <KamataEngine.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <dinput.h>
#include <string>

namespace {

constexpr float kTargetSeconds = 10.0f;
constexpr float kHideTimerSeconds = 7.0f;
constexpr int kPerfectScore = 1000;

} // namespace

void GameScene::Initialize() {
	KamataEngine::DebugText::GetInstance()->Initialize();

	// テストユーザーでログイン開始
	loginFuture_ = HttpClient::LoginAsync("submituser", "password123");

	phase_ = Phase::LoggingIn;
}

void GameScene::Update() {
	auto* const input = KamataEngine::Input::GetInstance();

	// -------------------------
	// ログイン中
	// -------------------------
	if (phase_ == Phase::LoggingIn) {

		if (loginFuture_.valid()) {

			const auto status = loginFuture_.wait_for(std::chrono::seconds(0));

			if (status == std::future_status::ready) {

				token_ = loginFuture_.get();

				if (!token_.empty()) {
					// ログイン成功
					phase_ = Phase::Ready;
				} else {
					// ログイン失敗
					phase_ = Phase::LoginFailed;
				}
			}
		}

		return;
	}

	if (phase_ == Phase::LoginFailed) {
		return;
	}

	// -------------------------
	// スコア送信中
	// -------------------------
	if (phase_ == Phase::Sending) {

		if (postFuture_.valid()) {

			const auto status = postFuture_.wait_for(std::chrono::seconds(0));

			// POST通信完了
			if (status == std::future_status::ready) {

				postSuccess_ = postFuture_.get();

				// POST完了後にランキング取得開始
				getFuture_ = HttpClient::GetScoresAsync();

				phase_ = Phase::LoadingRanking;
			}
		}

		return;
	}

	// -------------------------
	// ランキング取得中
	// -------------------------
	if (phase_ == Phase::LoadingRanking) {

		if (getFuture_.valid()) {

			const auto status = getFuture_.wait_for(std::chrono::seconds(0));

			// GET通信完了
			if (status == std::future_status::ready) {

				ranking_ = getFuture_.get();

				phase_ = Phase::Ranking;
			}
		}

		return;
	}

	// -------------------------
	// SPACE入力
	// -------------------------
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

	case Phase::LoggingIn:
	case Phase::LoginFailed:
	case Phase::Sending:
	case Phase::LoadingRanking:

		break;
	}
}

void GameScene::Draw() {
	auto* const debugText = KamataEngine::DebugText::GetInstance();

	KamataEngine::Sprite::PreDraw();

	debugText->Print("10 SECOND STOP CHALLENGE", 380.0f, 80.0f, 2.0f);

	switch (phase_) {

	case Phase::LoggingIn:

		debugText->Print("LOGGING IN...", 500.0f, 300.0f, 1.5f);

		debugText->Print("Please wait", 535.0f, 350.0f, 1.0f);

		break;

	case Phase::LoginFailed:

		debugText->Print("LOGIN FAILED", 500.0f, 300.0f, 1.5f);

		debugText->Print("Check username / password / server", 420.0f, 350.0f, 1.0f);

		break;

	// -------------------------
	// 開始待ち
	// -------------------------
	case Phase::Ready:

		debugText->Print("Press SPACE to start the timer", 420.0f, 260.0f, 1.5f);

		debugText->Print("Stop as close to 10.000 seconds as possible!", 360.0f, 320.0f, 1.2f);

		debugText->Print("The timer disappears after 7 seconds.", 400.0f, 360.0f, 1.0f);

		break;

	// -------------------------
	// ゲーム中
	// -------------------------
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

	// -------------------------
	// POST送信中
	// -------------------------
	case Phase::Sending:

		debugText->Print("SENDING SCORE...", 500.0f, 300.0f, 1.5f);

		debugText->Print("Please wait", 535.0f, 350.0f, 1.0f);

		break;

	// -------------------------
	// GETランキング取得中
	// -------------------------
	case Phase::LoadingRanking:

		debugText->Print("LOADING RANKING...", 490.0f, 300.0f, 1.5f);

		debugText->Print("Please wait", 535.0f, 350.0f, 1.0f);

		break;

	// -------------------------
	// 結果・ランキング
	// -------------------------
	case Phase::Ranking:

		debugText->Print("RESULT", 545.0f, 175.0f, 1.5f);

		debugText->Print("STOP TIME: " + std::to_string(stoppedTime_).substr(0, 6) + " sec", 450.0f, 225.0f, 1.2f);

		debugText->Print("SCORE: " + std::to_string(score_), 500.0f, 270.0f, 1.5f);

		// POST成功・失敗表示
		if (postSuccess_) {

			debugText->Print("SERVER: POST SUCCESS", 500.0f, 330.0f, 1.0f);

		} else {

			debugText->Print("SERVER: POST FAILED", 500.0f, 330.0f, 1.0f);
		}

		debugText->Print("SERVER TOP 5", 510.0f, 390.0f, 1.2f);

		// サーバーから取得したランキング
		for (std::size_t i = 0; i < ranking_.size(); ++i) {

			debugText->Print(std::to_string(i + 1) + ".  " + std::to_string(ranking_[i]) + " pts", 535.0f, 425.0f + static_cast<float>(i) * 28.0f, 1.0f);
		}

		debugText->Print("Press SPACE to retry", 500.0f, 600.0f, 1.2f);

		break;
	}

	debugText->DrawAll();

	KamataEngine::Sprite::PostDraw();
}

void GameScene::StartGame() {

	startTime_ = Clock::now();

	stoppedTime_ = 0.0f;

	score_ = 0;

	ranking_.clear();

	postSuccess_ = false;

	phase_ = Phase::Playing;
}

void GameScene::StopGame() {

	stoppedTime_ = GetElapsedSeconds();

	// -------------------------
	// スコア計算
	// 10.000秒に近いほど高得点
	// -------------------------

	const float difference = std::abs(kTargetSeconds - stoppedTime_);

	const int errorMilliseconds = static_cast<int>(std::lround(difference * 1000.0f));

	score_ = (std::max)(0, kPerfectScore - errorMilliseconds);

	// -------------------------
	// Web APIへスコアを非同期POST
	// -------------------------

	postSuccess_ = false;

	postFuture_ = HttpClient::PostScoreAsync(score_, token_);

	phase_ = Phase::Sending;
}

float GameScene::GetElapsedSeconds() const { return std::chrono::duration<float>(Clock::now() - startTime_).count(); }