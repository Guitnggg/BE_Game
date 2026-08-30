#pragma once

#include <future>
#include <string>

class HttpClient {
public:
	// スコアを非同期でWeb APIへPOSTする
	static std::future<bool> PostScoreAsync(const std::string& name, int score);
};