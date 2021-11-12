#pragma once
#include <fstream>
#include <sys/stat.h>
#include <iostream>
#include <ctime>

class Logger
{
  public:
      enum LogLevel {
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
    Logger& operator=(const Logger & logger);

  public:
    Logger(const std::string& logDir = "./logs", const std::string& logFile = "log.txt", LogLevel webservLogLevel = INFO);

    static std::string getTimestamp(void);

    static LogLevel parseLogLevel(const std::string& s);\

    void setWebservLogLevel(LogLevel logLevel);
    
    template <typename T>
    Logger& operator<<(const T& t)
    {
      if (_currentLogLevel >= _webservLogLevel) {
        _logStream << t;
        _logStream.flush();
      }

      return *this;
    }
    

    Logger& operator<<(LogLevel logLevel);
};

extern Logger glogger;