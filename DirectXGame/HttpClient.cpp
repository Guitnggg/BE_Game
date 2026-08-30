#include "HttpClient.h"

#include <curl/curl.h>
#include <future>
#include <string>
#include <algorithm>
#include <vector>

std::future<bool> HttpClient::PostScoreAsync(const std::string& name, int score) {
	return std::async(std::launch::async, [name, score]() -> bool {
		CURL* curl = curl_easy_init();

		if (!curl) {
			return false;
		}

		const std::string url = "http://localhost:3000/scores";

		const std::string json = "{"
		                         "\"name\":\"" +
		                         name +
		                         "\","
		                         "\"score\":" +
		                         std::to_string(score) + "}";

		curl_slist* headers = nullptr;

		headers = curl_slist_append(headers, "Content-Type: application/json");

		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

		curl_easy_setopt(curl, CURLOPT_POST, 1L);

		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());

		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

		// 通信が永遠に待ち状態にならないようにする
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);

		const CURLcode result = curl_easy_perform(curl);

		curl_slist_free_all(headers);
		curl_easy_cleanup(curl);

		return result == CURLE_OK;
	});
}

namespace {
size_t WriteCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
	const size_t totalSize = size * nmemb;

	auto* response = static_cast<std::string*>(userdata);

	response->append(ptr, totalSize);

	return totalSize;
}
} // namespace

std::future<std::vector<int>> HttpClient::GetScoresAsync() {
	return std::async(std::launch::async, []() -> std::vector<int> {
		std::vector<int> scores;

		CURL* curl = curl_easy_init();

		if (!curl) {
			return scores;
		}

		const std::string url = "http://localhost:3000/scores";

		std::string response;

		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);

		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);

		const CURLcode result = curl_easy_perform(curl);

		curl_easy_cleanup(curl);

		if (result != CURLE_OK) {
			return scores;
		}

		// -------------------------
		// JSON文字列からscoreを抽出
		// -------------------------

		const std::string key = "\"score\":";

		std::size_t pos = 0;

		while (true) {

			pos = response.find(key, pos);

			if (pos == std::string::npos) {
				break;
			}

			pos += key.length();

			try {
				const int score = std::stoi(response.substr(pos));

				scores.push_back(score);
			} catch (...) {
			}
		}

		// 高い順
		std::sort(scores.begin(), scores.end(), std::greater<int>());

		// 上位5件だけ
		if (scores.size() > 5) {
			scores.resize(5);
		}

		return scores;
	});
}