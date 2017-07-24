#include "stdafx.h"

Logger::Logger() : path(Utility::GetModuleName(NULL) + ".log")
{
}

void Logger::Write(const char * format, ...) const
{
	char inBuf[MAX_STRING];

	va_list va;

	va_start(va, format);

	vsprintf_s(inBuf, format, va);

	va_end(va);

	auto text = Utility::FormatString("[%s] [LOG] %s\n",
		Utility::GetShortTimeString().c_str(), inBuf);

	std::ofstream ofs(path, std::ios::app);

	ofs << text;

	ofs.close();

#ifdef DEBUG

	OutputDebugStringA(text.c_str());

#endif
}

void Logger::Remove() const
{
	if (!Utility::FileExists(path)) return;

	remove(path.c_str());
}

Logger::~Logger()
{
	//Remove();
}
