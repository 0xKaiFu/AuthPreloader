#include "Utils.hpp"
#include <algorithm>
#include <chrono>
#include <random>
#include <fstream>
#include <cstring>


std::vector<char> Utils::readFileRaw(const std::string& filePath)
{
	std::ifstream file(filePath, std::ios::binary);

	if (!file)
	{
		return {};
	}

	file.seekg(0, std::ios::end);
	std::streamsize fileSize = file.tellg();
	file.seekg(0, std::ios::beg);

	std::vector<char> buffer(fileSize);

	if (!file.read(buffer.data(), fileSize))
	{
		return {};
	}

	file.close();

	return buffer;
}

#include <nlohmann/json.hpp>

bool Utils::ValidJson(std::string _json) {
	try {
		nlohmann::json::parse(_json);
		return true;
	}
	catch (const nlohmann::json::parse_error& e) {
		return false;
	}
}
