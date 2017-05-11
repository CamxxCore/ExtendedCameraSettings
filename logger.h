#pragma once

#define LOG(x,...) g_logfile.Write(x, __VA_ARGS__);

class Logger sealed
{
public:
	Logger();
	Logger(std::string filename);
	~Logger();
	void Write(const char * format, ...) const;
	void Remove() const;
private:
	void Initialize(std::string name);
	std::string logName;
};

static Logger g_logfile;
