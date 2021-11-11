#include "webserv/Logger.hpp"
#include <sys/stat.h>
#include <dirent.h>

Logger::Logger(const std::string& logDir, const std::string& logFile): _logDir(logDir), _logFile(logFile)
{
	stat info;

	if (stat(logDir, &info) != 0) {
	}
}
