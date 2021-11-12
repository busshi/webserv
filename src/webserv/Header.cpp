#include "Header.hpp"
#include "Constants.hpp"
#include <sys/stat.h>
#include <dirent.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

Header::Header( void ) {}

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

void		Header::_parseFirstLine( std::string s, std::string root ) {

	unsigned pos = s.find(' ');
	_headerParam["Method"] = s.substr(0, pos);

	_headerParam["Root"] = root;
			
	unsigned pos2 = s.find(' ', pos + 1);
	_headerParam["Path"] = s.substr(pos + 1, pos2 - pos - 1);

	std::size_t	found = _headerParam["Path"].find_last_of('.');
	if (found != std::string::npos)
		_headerParam["Content-Type"] = _setContentType(_headerParam["Path"].substr(found + 1));

	_headerParam["HTTP"] = s.substr(pos2 + 1, s.length() - pos2 - 2);
}

void		Header::parseHeader(char buffer[], std::string rootPath) {

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
			_parseFirstLine(*it, rootPath);
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

	
	std::ofstream	ofs("asset/error_page.html");
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
			_genErrorPage("asset/sample_error.html", "500", "Internal Server Error", "the server encountered an internal error");
			path = "asset/error_page.html";

		}
		else {

			_headerParam["Status-Code"] = "404 Not Found";
			_genErrorPage("asset/sample_error.html", "404", "Not Found", "the page you are looking for doesn't exist");
			path = "asset/error_page.html";
		}
		
		path = "asset/error_page.html";
		ifs.open(path.c_str());
		_headerParam["Content-Type"] = "text/html";
	}
	
    buf << ifs.rdbuf();
    ifs.close();
}

void	Header::_autoIndexResponse( std::string path, std::stringstream & buf ) {

	struct stat	s;

	if (stat(path.c_str(), &s) == 0) {

		if (s.st_mode & S_IFDIR) {

			path = path.substr(0, path.find_last_of('/'));

			std::cout << ORANGE << "Autoindex is ON... Listing directory: " << path << CLR << std::endl;

			DIR *				folder = opendir(path.c_str());
		
			if (folder) {
			
				struct dirent *	dir;

				while ((dir = readdir(folder)) != NULL) {
				
					buf << "<a href=\"http://" << _headerParam["Host"] << "/" << dir->d_name << "\">" << dir->d_name << "</a><br/>" << std::endl;
					std::cout << dir->d_name << std::endl;
				}

				std::cout << std::endl;
				closedir(folder);
			}
		}
		else
			_noAutoIndexResponse(path, buf);
	}
}

void    	Header::createResponse( std::string autoindex ) {

	std::stringstream	buf;
    std::string 		path = _headerParam["Root"];

	if (_headerParam["Path"] == "/")
		path += "index.html";
    else
		path += _headerParam["Path"].substr(1);

	std::cout << "PAAAAAAAAAAAAATH=>" << path << std::endl;
	if (autoindex == "off")
		_noAutoIndexResponse(path, buf);
	else
		_autoIndexResponse(path, buf);

	unsigned len = buf.str().size();
	std::stringstream	tmp;
	tmp << len;
	_headerParam["Content-Length"] = tmp.str();
    
	_response =
          _headerParam["HTTP"] + " " + _headerParam["Status-Code"] + "\n" + 
		  "Content-Type: " + _headerParam["Content-Type"] + ";charset=UTF-8\n" + 
		  "Content-Length: " + _headerParam["Content-Length"] + "\n" +
			"Date: " + _getDate(time(0)) + "\n" +
			"Last-Modified: " + _getLastModified(path) + "\n" +
			"Location: " + _headerParam["Referer"] + "\n" +
			"Server: webserv©" + "\n\n" + 
		  buf.str();
}
