#include "HttpClient.h"

#include <algorithm>
#include <curl/curl.h>
#include <future>
#include <string>
#include <vector>

namespace {

// Web APIから返ってきたレスポンスをstringに保存する
size_t WriteCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
	const size_t totalSize = size * nmemb;

	auto* response = static_cast<std::string*>(userdata);

	response->append(ptr, totalSize);

	return totalSize;
}

} // namespace

// ============================================================
// ログイン
// ============================================================
std::future<std::string> HttpClient::LoginAsync(const std::string& name, const std::string& password) {

	return std::async(std::launch::async, [name, password]() -> std::string {
		CURL* curl = curl_easy_init();

		if (!curl) {
			return "";
		}

		// ログイン
		const std::string url = "https://swgame-six-eta.vercel.app/users/login";

		const std::string json = "{\"name\":\"" + name + "\",\"password\":\"" + password + "\"}";

		std::string response;

		curl_slist* headers = nullptr;

		headers = curl_slist_append(headers, "Content-Type: application/json");

		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

		curl_easy_setopt(curl, CURLOPT_POST, 1L);

		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());

		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);

		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

		// 通信が永遠に待ち状態にならないようにする
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);

		const CURLcode result = curl_easy_perform(curl);

		curl_slist_free_all(headers);
		curl_easy_cleanup(curl);

		if (result != CURLE_OK) {
			return "";
		}

		// -------------------------
		// JSONからtokenを取り出す
		// -------------------------

		const std::string key = "\"token\":\"";

		std::size_t start = response.find(key);

		if (start == std::string::npos) {
			return "";
		}

		start += key.length();

		const std::size_t end = response.find("\"", start);

		if (end == std::string::npos) {
			return "";
		}

		return response.substr(start, end - start);
	});
}

// ============================================================
// JWT付きスコア送信
// ============================================================
std::future<bool> HttpClient::PostScoreAsync(int score, const std::string& token) {

	return std::async(std::launch::async, [score, token]() -> bool {
		CURL* curl = curl_easy_init();

		if (!curl) {
			return false;
		}

		// スコア送信
		const std::string url = "https://swgame-six-eta.vercel.app/scores";

		// ユーザー名は送らない
		// JWTからサーバー側でユーザーを特定する
		const std::string json = "{\"score\":" + std::to_string(score) + "}";

		curl_slist* headers = nullptr;

		headers = curl_slist_append(headers, "Content-Type: application/json");

		// JWTをAuthorizationヘッダーに設定
		const std::string authHeader = "Authorization: Bearer " + token;

		headers = curl_slist_append(headers, authHeader.c_str());

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

// ============================================================
// スコアランキング取得
// ============================================================
std::future<std::vector<int>> HttpClient::GetScoresAsync() {

	return std::async(std::launch::async, []() -> std::vector<int> {
		std::vector<int> scores;

		CURL* curl = curl_easy_init();

		if (!curl) {
			return scores;
		}

		// スコアランキング取得
		const std::string url = "https://swgame-six-eta.vercel.app/scores";

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