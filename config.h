#pragma once

class CConfig
{
public:
	CConfig(const char* fileName) {
		char buffer[0x100];

		GetCurrentDirectory(MAX_PATH, buffer);

		strcat_s(buffer, "\\");

		strcat_s(buffer, fileName);

		strcpy_s(filename, sizeof(buffer), buffer);
	}

	template <typename T>
	T get(const char * section, const char * key, T defaultValue)
	{
		T result;

		char buffer[0x100];

		GetPrivateProfileStringA(section, key, "err", buffer, sizeof(buffer), filename);

		if (GetLastError() == 0x2 || strstr(buffer, "err")) return defaultValue;

		std::stringstream ss;

		ss << buffer;
		ss >> result;

		return result;
	}

private:

	char filename[MAX_PATH];
};

