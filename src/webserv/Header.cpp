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

	return s.substr(start);
}

void		Header::_parseFirstLine( std::string s, std::string rootPath ) {

	unsigned pos = s.find(' ');
	_headerParam["method"] = s.substr(0, pos);

	_headerParam["rootPath"] = rootPath;
			
	unsigned pos2 = s.find(' ', pos + 1);
	_headerParam["path"] = s.substr(pos + 1, pos2 - pos - 1);

	std::size_t	found = _headerParam["path"].find_last_of('.');
	if (found != std::string::npos)
		_headerParam["contentType"] = _setContentType(_headerParam["path"].substr(found + 1));

	_headerParam["http"] = s.substr(pos2 + 1);
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
			_headerParam["host"] = _setParam(*it);
		else if (line == 2)
			_headerParam["userAgent"] = _setParam(*it);
		else if (line == 3)
			_headerParam["accept"] = _setParam(*it);
		else if (line == 4)
			_headerParam["acceptLanguage"] = _setParam(*it);
		else if (line == 5)
			_headerParam["acceptEncoding"] = _setParam(*it);
		else if (line == 7)
			_headerParam["connection"] = _setParam(*it);
		else if (line == 8)
			_headerParam["referer"] = _setParam(*it);
	}
}

void    	Header::createResponse( void ) {
    std::ifstream ifs;
    std::string path = _headerParam["rootPath"];
	std::cout << "PATH" << path << std::endl;

	if (_headerParam["path"] == "/")
		path += "index.html";
    else
		path += _headerParam["path"].substr(1);

    ifs.open(path.c_str());
    if (ifs)
		_headerParam["statusCode"] = "200 OK";
	else {
		_headerParam["statusCode"] = "404 Not Found";
		path = "asset/default_404.html";
		ifs.open(path.c_str());
		_headerParam["contentType"] = "text/html";
	}

	std::stringstream	buf;
    buf << ifs.rdbuf();
    ifs.close();

	unsigned len = buf.str().size();
	std::stringstream	tmp;
	tmp << len;
	_headerParam["contentLen"] = tmp.str();
	
    _response =
          "HTTP/1.1 " + _headerParam["statusCode"] + "\n" + 
		  "Content-Type: " + _headerParam["contentType"] + ";charset=UTF-8\n" + 
		  "Content-Length: " + _headerParam["contentLen"] + "\n\n" + 
		  buf.str();
}
