#pragma once
#include <filesystem>
#include <iostream>

#include <curl/curl.h>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;

using json = nlohmann::json;
using jtype = nlohmann::json_abi_v3_12_0::detail::value_t;

inline size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

void LOG(std::string msg) {
	std::cout << msg << "\n";
}

extern bool showProgress;

inline int progress_callback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
	if (dltotal > 0 && showProgress) {
		double percentage = (double)dlnow / (double)dltotal * 100.0;
		printf("\rDownload progress: %.2f%%", percentage);
		fflush(stdout);
	}

	return 0;
}

inline bool POST(json& data, std::string type, json* returnData = nullptr) {
	std::string url = "https://internal-auth.katzu.dev/api";

	CURL* curl = curl_easy_init();
	if (!curl) {
		LOG("Failed to initialize CURL.");
		return false;
	}

	json req{};
	req["data"] = data;
	req["type"] = type;

	std::string json_payload = req.dump();

	std::string to_return;

	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_payload.c_str());
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, json_payload.length());

	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &write_callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &to_return);

	curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progress_callback);
	curl_easy_setopt(curl, CURLOPT_XFERINFODATA, nullptr);
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);

	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 300L);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);

	auto code = curl_easy_perform(curl);

	if (code != CURLE_OK) {
		LOG("CURL error: " + std::string(curl_easy_strerror(code)));
		curl_easy_cleanup(curl);
		return false;
	}

	try {
		if (!to_return.empty()) {
			nlohmann::json j = nlohmann::json::parse(to_return);

			if (!j.contains("ok") || j["ok"].type() != jtype::boolean) {
				curl_easy_cleanup(curl);
				LOG("Response does not match requirements...");
				return false;
			}

			if (j["ok"] != true) {
				if (j.contains("msg") && j["msg"].type() == jtype::string) {
					LOG(j["msg"]);
				}
				else {
					LOG("Unknown Error Ocurred.");
				}
				curl_easy_cleanup(curl);
				return false;
			}
		}
	}
	catch (const nlohmann::json::parse_error& e) {
		LOG("JSON parse error: " + std::string(e.what()));
		curl_easy_cleanup(curl);
		return false;
	}

	curl_easy_cleanup(curl);

	if (returnData != nullptr) {
		json data = json::parse(to_return);
		if (data.contains("data") && data["data"].type() == jtype::object)
			*returnData = json::parse(to_return)["data"];
		else
			*returnData = json::object();
	}
	return true;
}