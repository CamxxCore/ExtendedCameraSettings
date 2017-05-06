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
		T result{};

		TCHAR inBuf[0x100];

		if (!getText(inBuf, section, key))
			return defaultValue;

		std::stringstream sstream;

		sstream.imbue(std::locale("en-us"));

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

	bool getText(char * outBuffer, const char * section, const char * key) const
	{
		GetPrivateProfileString(TEXT(section),
			TEXT(key),
			NULL,
			outBuffer,
			sizeof(outBuffer),
			TEXT(filename));

		return outBuffer && GetLastError() == ERROR_SUCCESS;
	}

	template <typename T>
	void set(const char * section, const char * key, T val)
	{
		std::stringstream sstream;

		sstream.imbue(std::locale("en-us"));

		if (typeid(T) == typeid(bool))
		{
			sstream << std::boolalpha << val;
		}

		else
		{
			sstream << val;
		}
		
		WritePrivateProfileString (TEXT(section),
			                       TEXT(key),
			                       TEXT(sstream.str().c_str()),
			                       filename);
	}

	TCHAR filename[MAX_PATH];
};
