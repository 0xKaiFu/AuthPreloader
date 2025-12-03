#pragma once
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <random>

class Utils
{
public:
	static std::vector<char> readFileRaw(const std::string& filePath);
	static bool ValidJson(std::string _json);
};