#pragma once
#include <fstream>
#include <sys/stat.h>
#include <iostream>
#include <ctime>

// 17
class Logger
{
  private:
    std::ofstream _logStream;
    std::string _logDir, _logFile;

    Logger(const Logger& other);
    Logger& operator=(const Logger & logger);

  public:
      Logger(const std::string& logDir = "./logs", const std::string& logFile = "log.txt");

      static std::string getTimestamp(void);

  template <typename T>
  Logger& operator<<(const T& t)
  {
    _logStream << t;
    _logStream.flush();

    return *this;
  }
};

extern Logger glogger;