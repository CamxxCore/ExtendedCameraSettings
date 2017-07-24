#pragma once

#define LOG(x,...) g_logfile.Write(x, __VA_ARGS__);

class Logger sealed
{
public:
	explicit Logger(std::string filename) : path(filename) {}
	Logger();
	void Write(const char * format, ...) const;
	void Remove() const;
	~Logger();
private:
	const std::string path;
};

static Logger g_logfile;
