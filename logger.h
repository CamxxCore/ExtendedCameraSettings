#pragma once

class Logger
{
public:
	Logger(const char * fileName)
	{
		TCHAR inBuf[0x100];

		GetCurrentDirectory(MAX_PATH, inBuf);

		strcat_s(inBuf, "\\");

		strcat_s(inBuf, fileName);

		strcpy_s(filename, 0x100, inBuf);
	}

	void write(const char * fmt, ...)
	{
		va_list va;

		va_start(va, fmt);

		char buffer[0x100];

		vsprintf_s(buffer, fmt, va);

		va_end(va);

		tm t;

		time_t dtNow = time(NULL);

		localtime_s(&t, &dtNow);

		std::ofstream ofs(filename, std::ios::app);

		ofs.setf(std::ios::fixed, std::ios::floatfield);

		ofs.precision(2);

		ofs << '[' << t.tm_hour << ':' <<
			t.tm_min << ':' << t.tm_sec << "] " << buffer << "\n";

		ofs.close();
	}

private:
	TCHAR filename[MAX_PATH];
};
