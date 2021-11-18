#include "Header.hpp"
#include "Constants.hpp"
#include "logger/Logger.hpp"
#include "utils/string.hpp"
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

void		Header::parseHeader(char buffer[]) {

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

		}
		else {

			_headerParam["Status-Code"] = "404 Not Found";
			_genErrorPage("asset/sample_error.html", "404", "Not Found", "the page you are looking for doesn't exist");
		}
		
		path = "asset/error_page.html";
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

std::string	Header::_getIndex( std::string path, std::vector<std::string> indexes ) {

	std::string	index = "";
	std::string	file = "";
	struct stat	s;

	std::vector<std::string>::iterator	it, ite = indexes.end();
	
	for (it = indexes.begin(); it != ite; it++) {

		file = path + *it;
		glogger << Logger::DEBUG << "checking file + index => " << file << "\n";

		if (stat(file.c_str(), &s) == 0)
			return *it;
	}

	return index;
}

bool		Header::_isFolder( std::string path) {

	struct stat	s;

	if (stat(path.c_str(), &s) == 0) {

		if (s.st_mode & S_IFDIR)
			return true;
	}
	return false;
}

bool		Header::_haveLocation( std::string requestPath, std::string location ) {

	glogger << Logger::DEBUG << "requestPath [" << requestPath << "] [" << location << "] Location to search\n";

	size_t found = requestPath.find(location);
	glogger << Logger::DEBUG << "Found [" << found << "]\n";
	glogger << Logger::DEBUG << "npos [" << std::string::npos << "]\n";

	if (found == std::string::npos) {
		glogger << Logger::DEBUG << "DO NOT HAVE LOCATION\n";
		return false;
	}
	else {
		glogger << Logger::DEBUG << "HAVE LOCATION\n";
		return true;
	}
}

Header::Direc	Header::_getDirectives(ConfigItem * item, std::string suffix ) {

	Direc	directives;

	ConfigItem* root = item->findNearest("root");
	if (root) {
		directives.root = root->getValue();
		directives.path = directives.root.substr(0, directives.root.length() - 1) + suffix;
	}
	else {			
		directives.root = "none";
		glogger << Logger::WARNING << Logger::getTimestamp() << ORANGE << "Error: No default path provided!\n" << CLR;
	}

	ConfigItem* autoindex = item->findNearest("autoindex");
	if (autoindex)
		directives.autoindex = autoindex->getValue();
	else
		directives.autoindex = "off";

	ConfigItem *    index = item->findNearest("index");
   	if (index)  
         		directives.indexes = split(index->getValue());

	ConfigItem *    methods = item->findNearest("method");
   	if (methods)  
         		directives.methods = split(methods->getValue());

	ConfigItem *	uploadPath = item->findNearest("file_upload_dir");
	if (uploadPath)
		directives.uploadPath = uploadPath->getValue();
	else {
		directives.uploadPath = "tmp/";
		glogger << Logger::WARNING << Logger::getTimestamp() << ORANGE << " No upload path provided! Using default path: " << directives.uploadPath << "\n" << CLR;
	}

	ConfigItem *	defaultErrorFile = item->findNearest("default_error_file");
	if (defaultErrorFile)
		directives.defaultErrorFile = defaultErrorFile->getValue();
	else {
		glogger << Logger::WARNING << Logger::getTimestamp() << ORANGE << " No error file path provided! Using webserv default error pages\n" << CLR;
	}

	ConfigItem *	uploadMaxSize = item->findNearest("upload_max_size");
	if (uploadMaxSize)
		directives.uploadMaxSize = uploadMaxSize->getValue();
	else {
		glogger << Logger::WARNING << Logger::getTimestamp() << ORANGE << " No upload_max_size directive found! Using webserv default upload_max_size\n" << CLR;
	}

	ConfigItem *	bodyMaxSize = item->findNearest("client_body_max_size");
	if (bodyMaxSize)
		directives.bodyMaxSize = bodyMaxSize->getValue();
	else {
		glogger << Logger::WARNING << Logger::getTimestamp() << ORANGE << " No body_max_size directive found! Using webserv default body_max_size\n" << CLR;
	}

	return directives;
}

std::string	Header::_checkErrorPage( std::string defaultPage, std::string code, std::string errorMsg, std::string errorSentence ) {

	struct stat	s;

	if (stat(defaultPage.c_str(), &s) == 0) {

		glogger << Logger::DEBUG << Logger::getTimestamp() << " Default error page exists. Using " << defaultPage << "\n";
		return defaultPage;
	}
	else {
		
		glogger << Logger::DEBUG << Logger::getTimestamp() << " Default error page does not exist. Using webserv default error page\n";
		_genErrorPage("asset/sample_error.html", code, errorMsg, errorSentence);
		return "asset/error_page.html";
	}
}

void    	Header::createResponse( ConfigItem * item ) {

	Direc				directives;
	std::stringstream	buf;
	std::string			location;
	bool				haveLoc = false;

	std::vector<ConfigItem *>	locations = item->findBlocks("location");

	for (std::vector<ConfigItem*>::iterator it = locations.begin(); it != locations.end(); it++) {

		location = (*it)->getValue();

		if ((haveLoc = _haveLocation(_headerParam["Path"] + "/", location)) == true) {
			glogger << Logger::DEBUG << "haveLoc is true\n";
			directives = _getDirectives(*it, location);

		}
	}

	if (haveLoc == false) {

		glogger << Logger::DEBUG << "haveLoc is false\n";
		directives = _getDirectives(item, _headerParam["Path"]);
	}

	glogger << Logger::DEBUG << "_headerParam[\"Path\"] [" << _headerParam["Path"] << "]\n";
	glogger << Logger::DEBUG << "Root [" << directives.root << "]\n";
	glogger << Logger::DEBUG << "Path [" << directives.path << "]\n";
	glogger << Logger::DEBUG << "Location [" << location << "]\n";
	glogger << Logger::DEBUG << "Autoindex [" << directives.autoindex << "]\n";
	glogger << Logger::DEBUG << "Upload Path [" << directives.uploadPath << "]\n";
	glogger << Logger::DEBUG << "Default Error File [" << directives.defaultErrorFile << "]\n";
	glogger << Logger::DEBUG << "Upload Max Size [" << directives.uploadMaxSize << "]\n";
	glogger << Logger::DEBUG << "Body Max Size [" << directives.bodyMaxSize << "]\n";
	glogger << Logger::DEBUG << "Methods allowed [ ";
	for (size_t i = 0; i < directives.methods.size(); i++)
		glogger << Logger::DEBUG << directives.methods[i] << " ";
	glogger << Logger::DEBUG << "]\n";

	if (_isFolder(directives.path) == true) {
		glogger << Logger::DEBUG << "IS FOLDER\n";
		directives.path += _getIndex(directives.path, directives.indexes);

		glogger << Logger :: DEBUG << "Path+index [" << directives.path << "]\n";
		if (_isFolder(directives.path) == true) {

			if (directives.autoindex == "on")
				_autoIndexResponse(directives.path, buf);
			else {
				std::string	errorPage = _checkErrorPage(directives.defaultErrorFile, "403", "Forbidden", "the access is permanently forbidden");

				std::ifstream		ifs;
				ifs.open(errorPage.c_str());
    			buf << ifs.rdbuf();
    			ifs.close();

				_headerParam["Status-Code"] = "403 Forbidden";
				_headerParam["Content-Type"] = "text/html";
			}
		}
		else
			_noAutoIndexResponse(directives.path, buf);
	}
	else
		_noAutoIndexResponse(directives.path, buf);

	unsigned len = buf.str().size();
	std::stringstream	tmp;
	tmp << len;
	_headerParam["Content-Length"] = tmp.str();
    
	_response =
          _headerParam["HTTP"] + " " + _headerParam["Status-Code"] + "\r\n" + 
		  "Content-Type: " + _headerParam["Content-Type"] + ";charset=UTF-8\r\n" + 
		  "Content-Length: " + _headerParam["Content-Length"] + "\r\n" +
			"Date: " + _getDate(time(0)) + "\r\n" +
			"Last-Modified: " + _getLastModified(directives.path) + "\r\n" +
			"Location: " + _headerParam["Referer"] + "\r\n" +
			"Server: webserv" + "\r\n\r\n" + 
		  buf.str();
}
