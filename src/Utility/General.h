#pragma once

namespace Utility
{
	HMODULE GetActiveModule();

	std::string GetModuleName(HMODULE hModule);

	std::string GetWorkingDirectory();

	std::string GetShortTimeString();

	bool FileExists(std::string fileName);

	void SplitString(std::string str, std::string splitBy, std::vector<std::string>& tokens);

	void ToLower(std::string& str);

	unsigned int GetHashKey(std::string str);

	unsigned int StringToHash(std::string str);

	template<typename ... Args>
	std::string FormatString(const std::string& format, Args ... args)
	{
		size_t size = snprintf(nullptr, 0, format.c_str(), args ...) + 1;
		std::unique_ptr<char[]> buf(new char[size]);
		snprintf(buf.get(), size, format.c_str(), args ...);
		return std::string(buf.get(), buf.get() + size - 1);
	}
};
