#include "utils/Logger.hpp"
#include "utils/Formatter.hpp"
#include "utils/string.hpp"
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>

Logger::Logger(const std::string& logDir,
               const std::string& logFile,
               Logger::LogLevel webservLogLevel)
  : _logDir(logDir)
  , _logFile(logFile)
  , _webservLogLevel(webservLogLevel)
  , _currentLogLevel(INFO)
{
    struct stat info;

    /* create log directory in case it does not exist or isn't a directory */
    if (stat(logDir.c_str(), &info) != 0 || (info.st_mode & S_IFDIR) == 0) {
        unlink(logDir.c_str());
        mkdir(logDir.c_str(), 0755);
    }

    const std::string fullpath = (_logDir + "/" + _logFile);
    _logStream.open(fullpath.c_str());

    if (!_logStream) {
        std::cerr << "Logger: unrecoverable error: unable to open logFile "
                  << fullpath << ". (this logger won't be usable)\n";
    }
}

std::string
Logger::getTimestamp(void)
{
    time_t rawTime;
    char buf[1024];

    time(&rawTime);
    tm* timeinfo = localtime(&rawTime);

    buf[0] = '[';
    buf[strftime(buf + 1, 1024, "%Y-%m-%e-%H-%M-%S", timeinfo) + 1] = ']';

    return buf;
}

Logger::LogLevel
Logger::parseLogLevel(const std::string& s)
{
    std::string levelAsStr = toLowerCase(s);
    std::string levels[] = { "debug", "info", "warning", "error" };

    for (unsigned i = 0; i != sizeof(levels) / sizeof(*levels); ++i) {
        if (levelAsStr == levels[i]) {
            return static_cast<LogLevel>(i);
        }
    }

    return UNKNOWN;
}

Logger&
Logger::operator<<(LogLevel logLevel)
{
    _currentLogLevel = logLevel;

    return *this;
}

void
Logger::setWebservLogLevel(Logger::LogLevel logLevel)
{
    _webservLogLevel = logLevel;
}
