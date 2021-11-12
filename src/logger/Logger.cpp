#include "logger/Logger.hpp"
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>

Logger::Logger(const std::string& logDir, const std::string& logFile): _logDir(logDir), _logFile(logFile)
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
		std::cerr << "Logger: unrecoverable error: unable to open logFile " << fullpath << ". (this logger won't be usable)\n";
	}
}

std::string Logger::getTimestamp(void)
{
	time_t rawTime;
	char buf[1024];

	time(&rawTime);
	tm* timeinfo = localtime(&rawTime);

	buf[0] = '[';
	buf[strftime(buf + 1, 1024, "%Y-%m-%e-%H-%M-%S", timeinfo) + 1] = ']';

	return buf;
}