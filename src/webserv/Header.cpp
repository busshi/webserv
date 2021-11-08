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
		_headerMap = rhs._headerMap;
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
	std::string res = s.substr(start);
	std::cout << "[" << res << "]" << std::endl;
	return res;
}

void		Header::_parseFirstLine( std::string s, std::string rootPath ) {

	unsigned pos = s.find(' ');
	_headerMap["method"] = s.substr(0, pos);

	_headerMap["rootPath"] = rootPath;
			
	unsigned pos2 = s.find(' ', pos + 1);
	std::string	tmp = s.substr(pos + 1, pos2 - pos - 1);
	_headerMap["path"] = tmp;

	std::size_t	found = tmp.find_last_of('.');
	if (found != std::string::npos)
		_headerMap["contentType"] = _setContentType(_headerMap["path"].substr(found + 1));

	_headerMap["http"] = s.substr(pos2 + 1);
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
			_headerMap["host"] = _setParam(*it);
		else if (line == 2)
			_headerMap["userAgent"] = _setParam(*it);
		else if (line == 3)
			_headerMap["accept"] = _setParam(*it);
		else if (line == 4)
			_headerMap["acceptLanguage"] = _setParam(*it);
		else if (line == 5)
			_headerMap["acceptEncoding"] = _setParam(*it);
		else if (line == 7)
			_headerMap["connection"] = _setParam(*it);
		else if (line == 8)
			_headerMap["referer"] = _setParam(*it);
	}
}

void    	Header::createResponse( void ) {
    std::ifstream ifs;
    std::string path = _headerMap["rootPath"];;

	if (!_headerMap["path"].length())
		path += "index.html";
    else
		path += _headerMap["path"];

    ifs.open(path.c_str());
    if (ifs)
		_headerMap["statusCode"] = "200 OK";
	else {
		_headerMap["statusCode"] = "404 Not Found";
		path = "asset/default_404.html";
		ifs.open(path.c_str());
		_headerMap["contentType"] = "text/html";
	}

	std::stringstream	buf;
    buf << ifs.rdbuf();
    ifs.close();

	unsigned len = buf.str().size();
	std::stringstream	tmp;
	tmp << len;
	_headerMap["contentLen"] = tmp.str();
	
    _response =
          "HTTP/1.1 " + _headerMap["statusCode"] + "\n" + 
		  "Content-Type: " + _headerMap["contentType"] + ";charset=UTF-8\n" + 
		  "Content-Length: " + _headerMap["contentLen"] + "\n\n" + 
		  buf.str();
}
