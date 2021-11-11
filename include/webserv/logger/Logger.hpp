#pragma once
#include <fstream>
#include <direct.h>
#include <sys/stat.h>

class Logger
{
  private:
    std::ostream _logStream;
    std::string _logDir, _logFile;

  public:
      Logger(const std::string& logDir = "./logs", const std::string& logFile = "log.txt");

  template <typename T>
  operator<<(const T& t)
  {
    _logStream << t;
  }
};
