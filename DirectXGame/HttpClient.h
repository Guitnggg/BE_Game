#pragma once

#include <future>
#include <string>
#include <vector>

class HttpClient {
public:
	// スコアを非同期でWeb APIへPOSTする
	static std::future<bool> PostScoreAsync(const std::string& name, int score);

	// スコアを非同期でWeb APIからGETする
	static std::future<std::vector<int>> GetScoresAsync();
};