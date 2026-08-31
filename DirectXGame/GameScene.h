#pragma once

#include <chrono>
#include <future>
#include <string>
#include <vector>

/// <summary>
/// 10秒だと思った瞬間にタイマーを止めるゲーム。
/// </summary>
class GameScene {
public:
	void Initialize();
	void Update();
	void Draw();

private:
	enum class Phase { LoggingIn, LoginFailed, Ready, Playing, Sending, LoadingRanking, Ranking };

	using Clock = std::chrono::steady_clock;

	void StartGame();
	void StopGame();

	float GetElapsedSeconds() const;

	Phase phase_ = Phase::LoggingIn;

	Clock::time_point startTime_{};

	float stoppedTime_ = 0.0f;
	int score_ = 0;

	std::vector<int> ranking_;

	// ログイン結果（JWT）
	std::future<std::string> loginFuture_;

	// ログイン後に保持するJWT
	std::string token_;

	// 非同期POSTの結果
	std::future<bool> postFuture_;

	// 非同期GETの結果
	std::future<std::vector<int>> getFuture_;

	bool postSuccess_ = false;
};