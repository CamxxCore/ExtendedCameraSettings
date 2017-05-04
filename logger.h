#pragma once

#define LOG(x,...) g_logfile.Write(x, __VA_ARGS__);

class Logger sealed
{
public:
	Logger();
	Logger(std::string filename);
	~Logger();
	void Initialize(std::string name);
	void Write(const char * format, ...);
	void Remove();
private:
	std::string logName;
};

static Logger g_logfile;
