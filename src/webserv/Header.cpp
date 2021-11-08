#include "Header.hpp"

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
//	unsigned stop = 0;

//	unsigned found = s.find('\r');
//	if (found == s.length() - 1)
//		stop = found - 1;

//	if (trim == true)
//		stop++;
//	return (stop ? s.substr(start, stop) : s.substr(start));
	return s.substr(start);
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
/*	std::cout << "host[" << _headerParam["Host"] << "]" << std::endl;
	std::cout << "agent[" << _headerParam["User-Agent"] << "]" << std::endl;
	std::cout << "accept[" << _headerParam["Accept"] << "]" << std::endl;
	std::cout << "language[" << _headerParam["Accept-Language"] << "]" << std::endl;
	std::cout << "encoding[" << _headerParam["Accept-Encoding"] << "]" << std::endl;
	std::cout << "connection[" << _headerParam["Connection"] << "]" << std::endl;
	std::cout << "referer[" << _headerParam["Referer"] << "]" << std::endl;*/
}

std::string	Header::_getDate( void ) {

	time_t			now = time(0);
	struct tm		*date;
	char			buffer[29];

	date = gmtime(&now);
	strftime(buffer, 29, "%a, %d %b %Y %H:%M:%S GMT", date);
	return std::string(buffer);
}

void    	Header::createResponse( void ) {
    std::ifstream ifs;
    std::string path = _headerParam["Root"];

	if (_headerParam["Path"] == "/")
		path += "index.html";
    else
		path += _headerParam["Path"].substr(1);

    ifs.open(path.c_str());
    if (ifs)
		_headerParam["Status-Code"] = "200 OK";
	else {
		_headerParam["Status-Code"] = "404 Not Found";
		path = "asset/default_404.html";
		ifs.open(path.c_str());
		_headerParam["Content-Type"] = "text/html";
	}

	std::stringstream	buf;
    buf << ifs.rdbuf();
    ifs.close();

	unsigned len = buf.str().size();
	std::stringstream	tmp;
	tmp << len;
	_headerParam["Content-Length"] = tmp.str();
	
    _response =
          _headerParam["HTTP"] + " " + _headerParam["Status-Code"] + "\n" + 
		  "Content-Type: " + _headerParam["Content-Type"] + ";charset=UTF-8\n" + 
		  "Content-Length: " + _headerParam["Content-Length"] + "\n" + "Date: " + _getDate() + "\n\n" + 
		  buf.str();
}
