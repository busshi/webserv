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

void	Header::parseHeader(char buffer[]) {

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
				_contentType = _path.substr(found + 1);
        }
    }

    display();
}

void    Header::createResponse( void ) {
    std::ifstream ifs;
    std::string path;

    if (!_path.length())
        path = "asset/index.html";
    else
        path = "asset/" + _path;

    ifs.open(path.c_str());
    if (ifs) {
        std::string s;
        std::string tmp;

        while (getline(ifs, s)) {
            tmp += s;
            tmp += "\n";
        }
        ifs.close();

        unsigned len = tmp.length();
        std::ostringstream o;
        o << len;
        std::string convertedLen(o.str());

        ifs.open(path.c_str());

        _response =
          "HTTP/1.1 200 OK\nContent-Type: " + _contentType + ";charset=UTF-8\nContent-Length: ";
        _response += convertedLen;
        _response += "\n\n";
        while (getline(ifs, s)) {
            _response += s;
            _response += "\n";
        }
        ifs.close();
    } else {

        std::ifstream       ifs_error;
        std::stringstream   buf;

        ifs_error.open("asset/error_page.html", std::ifstream::in);
        buf << ifs_error.rdbuf();
        ifs_error.close();

        _response = "HTTP/1.1 404 Not Found\nContent-Type: text/html;charset=UTF-8\nContent-Length: ";
        _response += buf.str().size();
        _response += "\n\n";
        _response += "NOT FOUND\n";
//      _response += buf.str();
    }
}

void	Header::display( void ) {

	std::cout << "--------------------------" << std::endl;
	std::cout << "Method: " << _method << std::endl;
	std::cout << "Path: " << _path << std::endl;
	std::cout << "Content: " << _contentType << std::endl;
	std::cout << "Content-Lentgh: " << _contentLen << std::endl;
	std::cout << "--------------------------" << std::endl;
}
