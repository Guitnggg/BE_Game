#pragma once

#include <future>
#include <string>
#include <vector>

class HttpClient {
public:
	static std::future<std::string> LoginAsync(const std::string& name, const std::string& password);

	static std::future<bool> PostScoreAsync(int score, const std::string& token);

	static std::future<std::vector<int>> GetScoresAsync();
};