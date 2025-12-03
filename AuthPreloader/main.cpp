#include <iostream>
#include <string>
#include <fstream>
#include "Web.hpp"

#include "Utils.hpp"
#include "base64.h"

#pragma comment (lib, "crypt32.lib")
#pragma comment (lib, "ws2_32.lib")

bool showProgress = false;

class PreloaderConfig {
public:
	std::string token = "";
	size_t hash = 0;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(PreloaderConfig, token, hash)
};

int main()
{
	curl_global_init(CURL_GLOBAL_ALL);

	std::string appdata = getenv("APPDATA");
	std::string folderPath = appdata + "\\Sora\\";
	std::string configPath = folderPath + "preloaderconfig.json";
	std::string loaderPath = folderPath + "loader.exe";

	if (fs::exists(folderPath) || !fs::is_directory(folderPath))
		fs::create_directories(folderPath);

	static PreloaderConfig config{};

	auto writeCfg = [&]() {
		std::ofstream ofs(configPath);
		ofs << json(config).dump();
		ofs.close();
		};

	json data{};
	json returnData{};

	bool tokenValid = false;

	std::string username = "";
	std::string password = "";

	size_t newHash = 0;

	if (fs::exists(configPath) && fs::is_regular_file(configPath)) {
		auto data = Utils::readFileRaw(configPath);
		std::string dataStr{ data.data(), data.size() };

		if (!Utils::ValidJson(dataStr)) {
			LOG("Config contains Invalid json... Ignoring.");
			goto postConfigChecks;
		}

		config = json::parse(dataStr);
	}

postConfigChecks:

	data["token"] = config.token;

	if (config.token == "" || !POST(data, "CheckToken")) {
		std::cout << "Please enter your username:" << std::endl;
		std::cin >> username;

		std::cout << "Please enter your password:" << std::endl;
		std::cin >> password;

		system("cls");

		data = {};

		data["password"] = password;
		data["username"] = username;

		returnData = {};
		if (!POST(data, "Login", &returnData)) {
			system("pause");
			return -1;
		}

		config.token = returnData["token"];
		writeCfg();
	}

	data = {};
	returnData = {};

	data["token"] = config.token;

	if (!POST(data, "GetLoaderHash", &returnData)) {
		system("pause");
		return -1;
	}

	newHash = returnData["hash"];

	if (newHash != config.hash || !fs::exists(loaderPath)) { // download loader
		showProgress = true;

		returnData = {};

		if (!POST(data, "DownloadLoader", &returnData))
		{
			system("pause");
			return -1;
		}

		showProgress = false;

		std::ofstream ofs(loaderPath, std::ios::binary);
		ofs << base64_decode(returnData["loader"]);
		ofs.close();
	}

	config.hash = newHash;
	writeCfg();

	LOG("Loader Hash: " + std::to_string(newHash));

	STARTUPINFOA si = { sizeof(si) };
	PROCESS_INFORMATION pi = {};

	if (CreateProcessA(loaderPath.c_str(), 0, 0, 0, false, 0, 0, 0, &si, &pi)) {
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
	}

	system("cls");
	return 0;
}
