#include "Header.hpp"
#include <iostream>
#include <vector>
#include "fstream"
#include "sstream"

Header::Header( void ) {}

Header::~Header( void ) {}

Header::Header( Header const & src ) { *this = src; }

Header &	Header::operator=( Header const & rhs ) {

	if (this != & rhs) {

		_response = rhs._response;
		_statusCode = rhs._statusCode;
		_contentType = rhs._contentType;
		_contentLen = rhs._contentLen;
		_method = rhs._method;
		_path = rhs._path;
		_rootPath = rhs._rootPath;
	}
	return *this;
}

std::string	Header::getResponse( void ) { return _response; }

void		Header::setContentType( std::string contentType ) {

	if (contentType == "html" || contentType == "css" || contentType == "javascript" || contentType == "plain")
		_contentType = "text/" + contentType;
	else if (contentType == "jpeg" || contentType == "png" || contentType == "bmp")
		_contentType = "image/" + contentType;
	else
		_contentType = "text/plain";
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

        if (line == 0) {

			unsigned pos = (*it).find(' ');
			_method = (*it).substr(0, pos);

			_rootPath = rootPath;
			
			unsigned pos2 = (*it).find(' ', pos + 1);
			_path = (*it).substr(pos + 1, pos2 - pos - 1);
			_path = _path.substr(1);

			std::size_t	found = _path.find_last_of('.');
			if (found != std::string::npos)
				setContentType(_path.substr(found + 1));

			_http = (*it).substr(pos2 + 1);
		}
		else if (line == 1)
			_host = (*it).substr(6);
		else if (line == 2)
			_userAgent = (*it).substr(12);
		else if (line == 3)
			_accept = (*it).substr(8);
		else if (line == 4)
			_acceptLanguage = (*it).substr(17);
		else if (line == 5)
			_acceptEncoding = (*it).substr(16);
		else if (line == 7)
			_connection = (*it).substr(12);
		else if (line == 8)
			_referer = (*it).substr(9);
	}
}

void    	Header::createResponse( void ) {
    std::ifstream ifs;
    std::string path;

    if (!_path.length())
        path = _rootPath + "index.html";
    else
        path = _rootPath + _path;

    ifs.open(path.c_str());
    if (ifs)
		_statusCode = "200 OK";
	else {
		_statusCode = "404 Not Found";
		path = "asset/default_404.html";
		ifs.open(path.c_str());
		_contentType = "text/html";
	}

	std::stringstream	buf;
    buf << ifs.rdbuf();
    ifs.close();

	unsigned len = buf.str().size();
	std::stringstream	tmp;
	tmp << len;
	_contentLen = tmp.str();
	
    _response =
          "HTTP/1.1 " + _statusCode + "\n" + 
		  "Content-Type: " + _contentType + ";charset=UTF-8\n" + 
		  "Content-Length: " + _contentLen + "\n\n" + 
		  buf.str();
}
