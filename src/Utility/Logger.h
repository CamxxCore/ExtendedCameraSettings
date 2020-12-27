#pragma once

#define LOG(x,...) g_logfile.Write(x, __VA_ARGS__);

namespace Utility {
    class Logger sealed {
    public:
        explicit Logger(std::string filename) : path(filename) {}
        Logger();
        void Write(const char* format, ...) const;
        void Remove() const;
        ~Logger();
    private:
        const std::string path;
    };

}

static Utility::Logger g_logfile;