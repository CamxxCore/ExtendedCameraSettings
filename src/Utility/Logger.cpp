#include "stdafx.h"

namespace Utility {
    char inBuf[90112];

    Logger::Logger() : path(GetModuleName(nullptr) + ".log")
    { }

    void Logger::Write(const char* format, ...) const {

        va_list va;
        va_start(va, format);
        vsprintf_s(inBuf, format, va);
        va_end(va);

        std::ofstream ofs(path, std::ios::app);

        auto str = Utility::FormatString("[%s] [LOG] %s\n", GetShortTimeString().c_str(), inBuf);
        ofs << str;
        ofs.close();

#ifdef _DEBUG
        OutputDebugStringA(str.c_str());
#endif
    }

    void Logger::Remove() const {

        if (!FileExists(path)) return;

        remove(path.c_str());
    }

    Logger::~Logger() {

        Remove();
    }
}

