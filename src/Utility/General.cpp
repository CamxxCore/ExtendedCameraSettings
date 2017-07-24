#include "stdafx.h"

static HMODULE ourModule;

HMODULE Utility::GetActiveModule()
{
	HMODULE hModule = NULL;

	GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
		reinterpret_cast<LPCSTR>(&GetActiveModule),
		&hModule);

	return hModule;
}

std::string Utility::GetModuleName(HMODULE hModule)
{
	TCHAR inBuf[MAX_PATH];

	if (!hModule)
		hModule = GetActiveModule();

	GetModuleFileName(hModule, inBuf, MAX_PATH);

	auto str = std::string(inBuf);

	auto seperator = str.find_last_of("\\");

	if (seperator != std::string::npos)
		seperator += 1;

	return str.substr(seperator, str.find_last_of(".") - seperator);
}

std::string Utility::GetWorkingDirectory()
{
	HMODULE hModule = GetActiveModule();

	TCHAR inBuf[MAX_PATH];

	GetModuleFileName(hModule, inBuf, MAX_PATH);

	auto str = std::string(inBuf);

	auto seperator = str.find_last_of("\\");

	if (seperator != std::string::npos)
		seperator += 1;

	return str.substr(0, seperator);

}

std::string Utility::GetShortTimeString()
{
	time_t t = time(NULL);

	struct tm timeinfo;

	localtime_s(&timeinfo, &t);

	return FormatString("%02d:%02d:%02d", 
		timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
}

bool Utility::FileExists(std::string fileName)
{
	std::ifstream infile(fileName);
	return infile.good();
}

void Utility::SplitString(std::string str, std::string splitBy, std::vector<std::string>& tokens)
{
	tokens.push_back(str);

	auto splitLen = splitBy.size();

	while (true)
	{
		auto frag = tokens.back();

		auto splitAt = frag.find(splitBy);

		if (splitAt == std::string::npos)
		{
			break;
		}

		tokens.back() = frag.substr(0, splitAt);

		tokens.push_back(frag.substr(splitAt + splitLen, frag.size() - (splitAt + splitLen)));
	}
}

void Utility::ToLower(std::string& str)
{
	transform(str.begin(), str.end(), str.begin(), std::tolower);
}

unsigned Utility::GetHashKey(std::string str)
{
	unsigned int hash = 0;
	for (int i = 0; i < str.size(); ++i)
	{
		hash += str[i];
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}
	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);
	return hash;
}

unsigned int Utility::StringToHash(std::string str)
{
	char * p;

	unsigned long numericHash = strtoul(str.c_str(), &p, 10);

	return *p != '\0' ? GetHashKey(str) : 

	static_cast<unsigned int>(numericHash);
}
