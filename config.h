#pragma once

#include <sstream>

class CConfig
{
public:
	CConfig(const char* fileName) {

		TCHAR inBuf[0x100];

		GetCurrentDirectory(MAX_PATH, inBuf);

		strcat_s(inBuf, "\\");

		strcat_s(inBuf, fileName);

		strcpy_s(filename, 0x100, inBuf);
	}

	template <typename T>
	T get(const char * section, const char * key, T defaultValue)
	{
		T result;

		TCHAR inBuf[0x100];

		GetPrivateProfileString (TEXT(section),
			                     TEXT(key),
			                     TEXT("err"),
			                     inBuf,
			                     0x100,
			                     TEXT(filename));

		if (GetLastError() != ERROR_SUCCESS|| !strcmp(inBuf, "err"))
		{
			return defaultValue;
		}

		std::stringstream sstream;

		if (typeid(T) == typeid(bool))
		{
			sstream << std::boolalpha << inBuf;
		}

		else
		{
			sstream << inBuf;
		}

		sstream >> result;

		return result;
	}

	template <typename T>
	void set(const char * section, const char * key, T val)
	{
		std::stringstream sstream;

		if (typeid(T) == typeid(bool))
		{
			sstream << std::boolalpha << val;
		}

		else
		{
			sstream << val;
		}

		std::string strResult = sstream.str();
		
		WritePrivateProfileString (TEXT(section),
			                       TEXT(key),
			                       TEXT(strResult.c_str()),
			                       filename);
	}

	TCHAR filename[MAX_PATH];
};

