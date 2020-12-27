#pragma once

#include <sstream>

class CConfig {
  public:
    CConfig(std::string fileName) : m_filename(fileName) {}
	CConfig() : CConfig(Utility::GetWorkingDirectory() + Utility::GetModuleName(nullptr) + ".ini") { }

    bool getText(char * outBuffer, const char * section, const char * key) const {
        GetPrivateProfileString(TEXT(section),
                                TEXT(key),
                                NULL,
                                outBuffer,
                                sizeof(outBuffer),
                                TEXT(m_filename.c_str()));

        return outBuffer &&
               GetLastError() == ERROR_SUCCESS;
    }

    void setText(const char * section, const char * key, const char * value) const {
        WritePrivateProfileString(TEXT(section),
                                  TEXT(key),
                                  TEXT(value),
                                  m_filename.c_str());
    }

    template <typename T>
    T get(const char * section, const char * key, T defaultValue) {
        T result{};

        TCHAR inBuf[80];

        if (!getText(inBuf, section, key))
            return defaultValue;

        std::stringstream sstream;

        sstream.imbue(std::locale("en-us"));

        if (typeid(T) == typeid(bool)) {
            sstream << std::boolalpha << inBuf;
        }

        else {
            sstream << inBuf;
        }

        sstream >> result;

        return result;
    }

    template <typename T>
    void set(const char * section, const char * key, T val) {
        std::stringstream sstream;

        sstream.imbue(std::locale("en-us"));

        if (typeid(T) == typeid(bool)) {
            sstream << std::boolalpha << val;
        }

        else {
            sstream << val;
        }

        setText(section, key, sstream.str().c_str());
    }

	std::string getPath() {
		return m_filename;
    }

    const std::string m_filename;
};
