#pragma once

class Utility
{
public:
	static HMODULE GetActiveModule();

	static std::string GetModuleName(HMODULE hModule);

	static std::string GetWorkingDirectory();

	static std::string GetShortTimeString();

	static bool FileExists(std::string fileName);

	static void SplitString(std::string str, std::string splitBy, std::vector<std::string>& tokens);
};
