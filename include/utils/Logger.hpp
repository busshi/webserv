#pragma once
#include <ctime>
#include <fstream>
#include <iostream>
#include <sys/stat.h>

class Logger
{
  public:
    enum LogLevel
    {
        UNKNOWN = -1,
        DEBUG,
        INFO,
        WARNING,
        ERROR
    };

  private:
    std::ofstream _logStream;
    std::string _logDir, _logFile;
    LogLevel _webservLogLevel, _currentLogLevel;

    Logger(const Logger& other);
    Logger& operator=(const Logger& logger);

  public:
    Logger(const std::string& logDir = "./logs",
           const std::string& logFile = "log.txt",
           LogLevel webservLogLevel = INFO);

    static std::string getTimestamp(void);

    static LogLevel parseLogLevel(const std::string& s);

    void setWebservLogLevel(LogLevel logLevel);

    LogLevel getWebservLogLevel(void) const;
    std::string getLogPath(void) const;

    template<typename T>
    Logger& operator<<(const T& t)
    {
#ifdef LOGGER
        if (_currentLogLevel >= _webservLogLevel) {
            _logStream << t;
            _logStream.flush();
        }
#else
        (void)t;
#endif

        return *this;
    }

    Logger& operator<<(LogLevel logLevel);
};
