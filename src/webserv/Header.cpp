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
	}
	return *this;
}

std::string	Header::getResponse( void ) { return _response; }

void		Header::setContentType( std::string contentType ) {

	if (contentType == "html" || contentType == "css" || contentType == "javascript" || contentType == "plain")
		_contentType = "text/" + contentType;
	else if (contentType == "jpeg" || contentType == "png" || contentType == "bmp")
		_contentType = "image/" + _contentType;
	else
		_contentType = "text/plain";
}

void		Header::parseHeader(char buffer[]) {

    std::vector<std::string> strings;
    std::istringstream buf(buffer);
    std::string s;

    while (getline(buf, s, ' '))
        strings.push_back(s);

    for (std::vector<std::string>::iterator it = strings.begin();
         it != strings.end();
         it++) {
        unsigned id = it - strings.begin();

        if (id == 0)
            _method = *it;
        if (id == 1) {
            _path = *it;
            _path = _path.substr(1);

			std::size_t	found = _path.find_last_of('.');
			if (found != std::string::npos)
				setContentType(_path.substr(found + 1));
        }
    }
}

void    	Header::createResponse( void ) {
    std::ifstream ifs;
    std::string path;

    if (!_path.length())
        path = "asset/index.html";
    else
        path = "asset/" + _path;

    ifs.open(path.c_str());
    if (ifs)
		_statusCode = "200 OK";
	else {
		_statusCode = "404 Not Found";
		path = "asset/error_page.html";
		ifs.open(path.c_str());
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
