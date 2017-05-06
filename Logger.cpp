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
	char inBuff[0x100];

	va_list va;

	va_start(va, format);

	vsprintf_s(inBuff, format, va);

	va_end(va);

	OutputDebugStringA(inBuff);

	std::ofstream ofs(logName, std::ios::app);

	ofs << '[' << Utility::GetShortTimeString() << "] [LOG] " << inBuff << "\n";

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
