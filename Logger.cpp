#include "stdafx.h"

Logger::Logger(std::string name)
{
	Initialize(name);
}

Logger::Logger()
{
	Initialize(Utility::GetModuleName(Utility::GetActiveModule()));
}

void Logger::Initialize(std::string name)
{
	logName = Utility::GetWorkingDirectory() + "\\" + name + ".log";

	Remove();
}

void Logger::Write(const char * format, ...) const
{
	char inBuff[MAX_STRING];

	va_list va;

	va_start(va, format);

	vsprintf_s(inBuff, format, va);

	va_end(va);

	char szText[MAX_STRING];

	sprintf_s(szText, "[%s] [LOG] %s\n", Utility::GetShortTimeString().c_str(), inBuff);

	OutputDebugStringA(szText);

	std::ofstream ofs(logName, std::ios::app);

	ofs << szText;

	ofs.close();
}

void Logger::Remove() const
{
	if (!Utility::FileExists(logName)) 
		return;

	std::remove(logName.c_str());
}

Logger::~Logger()
{
	Remove();
}
