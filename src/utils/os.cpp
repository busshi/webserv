#include "utils/os.hpp"
#include <sys/stat.h>

bool		isFolder( std::string path) {

	struct stat	s;

	if (stat(path.c_str(), &s) == 0) {

		if (s.st_mode & S_IFDIR)
			return true;
	}
	return false;
}

std::string	getDate( time_t timestamp ) {

	struct tm		*date;
	char			buffer[30];

	date = gmtime(&timestamp);
	strftime(buffer, 30, "%a, %d %b %Y %H:%M:%S GMT", date);

	return std::string(buffer);
}

std::string	getLastModified( std::string path ) {

	struct stat	res;

	if (stat(path.c_str(), &res) == 0)
		return getDate(res.st_mtime);
	else
		return getDate(time(0));
}

