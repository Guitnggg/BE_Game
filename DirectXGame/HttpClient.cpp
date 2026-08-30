#include "HttpClient.h"

#include <curl/curl.h>
#include <future>
#include <string>

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