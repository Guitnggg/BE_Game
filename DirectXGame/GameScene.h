#pragma once

#include <chrono>
#include <vector>

/// <summary>10秒だと思った瞬間にタイマーを止めるゲーム。</summary>
class GameScene {
public:
	void Initialize();
	void Update();
	void Draw();

private:
	enum class Phase { Ready, Playing, Ranking };
	using Clock = std::chrono::steady_clock;

	void StartGame();
	void StopGame();
	void SubmitScore();
	float GetElapsedSeconds() const;

	Phase phase_ = Phase::Ready;
	Clock::time_point startTime_{};
	float stoppedTime_ = 0.0f;
	int score_ = 0;
	std::vector<int> ranking_;
};
