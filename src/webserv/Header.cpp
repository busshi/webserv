#include "Header.hpp"
#include "Directives.hpp"
#include "Constants.hpp"
#include "logger/Logger.hpp"
#include "utils/string.hpp"
#include <sys/stat.h>
#include <dirent.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

Header::Header( char buffer[] ) {

    std::vector<std::string> lines;
    std::istringstream buf(buffer);
    std::string s;

	while (getline(buf, s))
		lines.push_back(s);

    for (std::vector<std::string>::iterator it = lines.begin();
         it != lines.end();
         it++) {
        unsigned line = it - lines.begin();

        if (line == 0)
			_parseFirstLine(*it);
		else if (line == 1)
			_headerParam["Host"] = _setParam(*it);
		else if (line == 2)
			_headerParam["User-Agent"] = _setParam(*it);
		else if (line == 3)
			_headerParam["Accept"] = _setParam(*it);
		else if (line == 4)
			_headerParam["Accept-Language"] = _setParam(*it);
		else if (line == 5)
			_headerParam["Accept-Encoding"] = _setParam(*it);
		else if (line == 7)
			_headerParam["Connection"] = _setParam(*it);
		else if (line == 8)
			_headerParam["Referer"] = _setParam(*it);
	}
}

Header::~Header( void ) {}

Header::Header( Header const & src ) { *this = src; }

Header &	Header::operator=( Header const & rhs ) {

	if (this != & rhs) {

		_response = rhs._response;
		_headerParam = rhs._headerParam;
	}
	return *this;
}

std::string	Header::getResponse( void ) { return _response; }

std::string	Header::_setContentType( std::string contentType ) {

	if (contentType == "html" || contentType == "css" || contentType == "javascript" || contentType == "plain")
		return "text/" + contentType;
	else if (contentType == "jpeg" || contentType == "png" || contentType == "bmp")
		return "image/" + contentType;
	else
		return "text/plain";
}

std::string	Header::_setParam( std::string s ) {

	unsigned start = s.find(' ') + 1;

	unsigned found = s.find("\r");

	return s.substr(start, found - start);
}

void		Header::_parseFirstLine( std::string s ) {

	unsigned pos = s.find(' ');
	_headerParam["Method"] = s.substr(0, pos);

	unsigned pos2 = s.find(' ', pos + 1);
	_headerParam["Path"] = s.substr(pos + 1, pos2 - pos - 1);

	std::size_t	found = _headerParam["Path"].find_last_of('.');
	if (found != std::string::npos)
		_headerParam["Content-Type"] = _setContentType(_headerParam["Path"].substr(found + 1));

	_headerParam["HTTP"] = s.substr(pos2 + 1, s.length() - pos2 - 2);
}

std::string	Header::_getDate( time_t timestamp ) {

	struct tm		*date;
	char			buffer[30];

	date = gmtime(&timestamp);
	strftime(buffer, 30, "%a, %d %b %Y %H:%M:%S GMT", date);
	
	return std::string(buffer);
}

std::string	Header::_getLastModified( std::string path ) {

	struct stat	res;

	if (stat(path.c_str(), &res) == 0)
		return _getDate(res.st_mtime);
	else
		return _getDate(time(0));
}

std::string		Header::_replace(std::string in, std::string s1, std::string s2) {

	std::string		out;
	std::string		line;
	size_t			len = s1.size();
	std::istringstream	iss(in);

	while (getline(iss, line)) {

		for (size_t i = 0; i < line.size(); i++) {
				
				size_t	found = line.find(s1, i);

				if (found == std::string::npos) {

					out += &line[i];
					break;
				}
				else {

					if (found > i) {

						std::string	subLine = line.substr(i, found - i);
						out += subLine;
						i += subLine.size();
					}
					out += s2;
					i += len - 1;
				}
		}
		out += "\n";
	}
	return out;
}

void		Header::_genErrorPage( std::string file, std::string code, std::string msg, std::string sentence ) {

	std::ifstream	ifs(file.c_str());
	std::string		res;
	std::string		line;
	std::string		out;

	while (getline(ifs, line)) {

		res += line;
		res += "\n";
	}

	ifs.close();

	out = _replace(res, "ERROR_CODE", code);
	out = _replace(out, "ERROR_MESSAGE", msg);
	out = _replace(out, "ERROR_SENTENCE", sentence);

	
	std::ofstream	ofs(ERROR_PAGE);
	ofs << out;
	ofs.close();
}

void	Header::_noAutoIndexResponse( std::string path, std::stringstream & buf ) {

	std::ifstream		ifs;

  	ifs.open(path.c_str());
	if (ifs)
		_headerParam["Status-Code"] = "200 OK";

	else {
	
		if (_headerParam["Root"] == "none") {
		
			_headerParam["Status-Code"] = "500 Internal Server Error";
			_genErrorPage(ERROR_SAMPLE, "500", "Internal Server Error", "the server encountered an internal error");

		}
		else {

			_headerParam["Status-Code"] = "404 Not Found";
			_genErrorPage(ERROR_SAMPLE, "404", "Not Found", "the page you are looking for doesn't exist");
		}
		
		path = ERROR_PAGE;
		ifs.open(path.c_str());
		_headerParam["Content-Type"] = "text/html";
	}
	
    buf << ifs.rdbuf();
    ifs.close();
}

void	Header::_autoIndexResponse( std::string path, std::stringstream & buf ) {

	glogger << Logger::DEBUG << PURPLE << "Autoindex is ON... Listing directory: " << path << CLR << "\n";

	DIR *				folder = opendir(path.c_str());
		
	if (folder) {
			
		struct dirent *	dir;

		while ((dir = readdir(folder)) != NULL) {
				
			buf << "<a href=\"http://" << _headerParam["Host"] << "/" << dir->d_name << "\">" << dir->d_name << "</a><br/>" << std::endl;
			glogger << Logger::DEBUG << dir->d_name << "\n";
		}

		glogger << Logger::DEBUG << "\n";
		closedir(folder);
	}
}

bool		Header::_isFolder( std::string path) {

	struct stat	s;

	if (stat(path.c_str(), &s) == 0) {

		if (s.st_mode & S_IFDIR)
			return true;
	}
	return false;
}

std::string	Header::_checkErrorPage( std::string defaultPage, std::string code, std::string errorMsg, std::string errorSentence ) {

	struct stat	s;

	if (stat(defaultPage.c_str(), &s) == 0) {

		glogger << Logger::DEBUG << Logger::getTimestamp() << " Default error page exists. Using " << defaultPage << "\n";
		return defaultPage;
	}
	else {
		
		glogger << Logger::DEBUG << Logger::getTimestamp() << " Default error page does not exist. Using webserv default error page\n";
		_genErrorPage(ERROR_SAMPLE, code, errorMsg, errorSentence);
		return ERROR_PAGE;
	}
}

void    	Header::createResponse( ConfigItem * item ) {

	Directives			directives;
	std::stringstream	buf;
	std::string			location;
	bool				haveLoc = false;

	std::vector<ConfigItem *>	locations = item->findBlocks("location");

	for (std::vector<ConfigItem*>::iterator it = locations.begin(); it != locations.end(); it++) {

		location = (*it)->getValue();

		if ((haveLoc = directives.haveLocation(_headerParam["Path"] + "/", location)) == true) {
			glogger << Logger::DEBUG << "haveLoc is true\n";
			directives.getConfig(*it, location);
		}
	}

	if (haveLoc == false) {

		glogger << Logger::DEBUG << "haveLoc is false\n";
		directives.getConfig(item, _headerParam["Path"]);
	}

	glogger << Logger::DEBUG << "_headerParam[\"Path\"] [" << _headerParam["Path"] << "]\n";
	glogger << Logger::DEBUG << "Root [" << directives.getRoot() << "]\n";
	glogger << Logger::DEBUG << "Path [" << directives.getPath() << "]\n";
	glogger << Logger::DEBUG << "Location [" << location << "]\n";
	glogger << Logger::DEBUG << "Autoindex [" << directives.getAutoIndex() << "]\n";
	glogger << Logger::DEBUG << "Upload Path [" << directives.getUploadPath() << "]\n";
	glogger << Logger::DEBUG << "Default Error File [" << directives.getDefaultErrorFile() << "]\n";
	glogger << Logger::DEBUG << "Upload Max Size [" << directives.getUploadMaxSize() << "]\n";
	glogger << Logger::DEBUG << "Body Max Size [" << directives.getBodyMaxSize() << "]\n";
//	glogger << Logger::DEBUG << "Methods allowed [ ";
//	std::vector<std::string>::iterator	it, ite = directives.getMethods().end();
//	for (it = directives.getMethods().begin(); it != ite; it++)
//		std::cout << *it << " ";
		//glogger << Logger::DEBUG << *it << " ";
//	glogger << Logger::DEBUG << "]\n";

	if (_isFolder(directives.getPath()) == true) {
		glogger << Logger::DEBUG << "IS FOLDER\n";
		directives.setPathWithIndex();

		glogger << Logger :: DEBUG << "Path+index [" << directives.getPath() << "]\n";
		if (_isFolder(directives.getPath()) == true) {

			if (directives.getAutoIndex() == "on")
				_autoIndexResponse(directives.getPath(), buf);
			else {
				std::string	errorPage = _checkErrorPage(directives.getDefaultErrorFile(), "403", "Forbidden", "the access is permanently forbidden");

				std::ifstream		ifs;
				ifs.open(errorPage.c_str());
    			buf << ifs.rdbuf();
    			ifs.close();

				_headerParam["Status-Code"] = "403 Forbidden";
				_headerParam["Content-Type"] = "text/html";
			}
		}
		else
			_noAutoIndexResponse(directives.getPath(), buf);
	}
	else
		_noAutoIndexResponse(directives.getPath(), buf);

	unsigned len = buf.str().size();
	std::stringstream	tmp;
	tmp << len;
	_headerParam["Content-Length"] = tmp.str();
    
	_response =
          _headerParam["HTTP"] + " " + _headerParam["Status-Code"] + "\r\n" + 
		  "Content-Type: " + _headerParam["Content-Type"] + ";charset=UTF-8\r\n" + 
		  "Content-Length: " + _headerParam["Content-Length"] + "\r\n" +
			"Date: " + _getDate(time(0)) + "\r\n" +
			"Last-Modified: " + _getLastModified(directives.getPath()) + "\r\n" +
			"Location: " + _headerParam["Referer"] + "\r\n" +
			"Server: webserv" + "\r\n\r\n" + 
		  buf.str();
}
