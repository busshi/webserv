#include "Directives.hpp"
#include "utils/Logger.hpp"

Directives::Directives(void) {}

Directives::~Directives(void) {}

Directives::Directives(const Directives& src)
{
    *this = src;
}

Directives&
Directives::operator=(const Directives& rhs)
{

    if (this != &rhs) {

        _root = rhs._root;
        _path = rhs._path;
        _autoindex = rhs._autoindex;
        _uploadPath = rhs._uploadPath;
        _defaultErrorFile = rhs._defaultErrorFile;
        _uploadMaxSize = rhs._uploadMaxSize;
        _bodyMaxSize = rhs._bodyMaxSize;
        _indexes = rhs._indexes;
        _methods = rhs._methods;
    }

    return *this;
}

std::string
Directives::getRoot(void)
{
    return _root;
}

std::string
Directives::getPath(void)
{
    return _path;
}

std::string
Directives::getAutoIndex(void)
{
    return _autoindex;
}

std::string
Directives::getUploadPath(void)
{
    return _uploadPath;
}

std::string
Directives::getDefaultErrorFile(void)
{
    return _defaultErrorFile;
}

std::string
Directives::getUploadMaxSize(void)
{
    return _uploadMaxSize;
}

std::string
Directives::getBodyMaxSize(void)
{
    return _bodyMaxSize;
}

std::vector<std::string>
Directives::getIndexes(void)
{
    return _indexes;
}

std::vector<std::string>
Directives::getMethods(void)
{
    return _methods;
}

void
Directives::getConfig(ConfigItem* item, std::string suffix)
{

    ConfigItem* root = item->findNearest("root");
    if (root) {
        _root = root->getValue();
        _path = _root.substr(0, _root.length() - (_root == "/")) + suffix;
    } else {
        _root = "none";
        glogger << Logger::WARNING << Logger::getTimestamp() << ORANGE
                << "Error: No default path provided!\n"
                << CLR;
    }

    ConfigItem* autoindex = item->findNearest("autoindex");
    if (autoindex)
        _autoindex = autoindex->getValue();
    else
        _autoindex = "off";

    ConfigItem* index = item->findNearest("index");
    if (index)
        _indexes = split(index->getValue());

    ConfigItem* methods = item->findNearest("method");
    if (methods)
        _methods = split(methods->getValue());

    ConfigItem* uploadPath = item->findNearest("file_upload_dir");
    if (uploadPath)
        _uploadPath = uploadPath->getValue();
    else {
        _uploadPath = UPLOAD_PATH;
        glogger << Logger::WARNING << Logger::getTimestamp() << ORANGE
                << " No upload path provided! Using default path: "
                << _uploadPath << "\n"
                << CLR;
    }

    ConfigItem* defaultErrorFile = item->findNearest("default_error_file");
    if (defaultErrorFile)
        _defaultErrorFile = defaultErrorFile->getValue();
    else {
        glogger
          << Logger::WARNING << Logger::getTimestamp() << ORANGE
          << " No error file path provided! Using webserv default error pages\n"
          << CLR;
    }

    ConfigItem* uploadMaxSize = item->findNearest("upload_max_size");
    if (uploadMaxSize)
        _uploadMaxSize = uploadMaxSize->getValue();
    else {
        glogger << Logger::WARNING << Logger::getTimestamp() << ORANGE
                << " No upload_max_size directive found! Using webserv default "
                   "upload_max_size\n"
                << CLR;
    }

    ConfigItem* bodyMaxSize = item->findNearest("client_body_max_size");
    if (bodyMaxSize)
        _bodyMaxSize = bodyMaxSize->getValue();
    else {
        glogger << Logger::WARNING << Logger::getTimestamp() << ORANGE
                << " No body_max_size directive found! Using webserv default "
                   "body_max_size\n"
                << CLR;
    }

    ConfigItem* cgiPass = item->findNearest("cgi_pass");
    if (cgiPass) {
        std::vector<std::string> components = split(cgiPass->getValue());
        _cgiPass.cgiExec = components.back();
        _cgiPass.exts = split(components.front(), ",");
    }
}

void
Directives::setPathWithIndex(void)
{

    std::string file = "";
    struct stat s;

    std::vector<std::string>::iterator it, ite = _indexes.end();

    for (it = _indexes.begin(); it != ite; it++) {

        file = _path + *it;
        glogger << Logger::DEBUG << "checking file + index => " << file << "\n";

        if (stat(file.c_str(), &s) == 0)
            _path += *it;
    }
}

bool
Directives::haveLocation(std::string requestPath, std::string location)
{

    glogger << Logger::DEBUG << "requestPath [" << requestPath << "] ["
            << location << "] Location to search\n";

    size_t found = requestPath.find(location);
    glogger << Logger::DEBUG << "Found [" << found << "]\n";
    glogger << Logger::DEBUG << "npos [" << std::string::npos << "]\n";

    if (found == std::string::npos) {
        glogger << Logger::DEBUG << "DO NOT HAVE LOCATION\n";
        return false;
    } else {
        glogger << Logger::DEBUG << "HAVE LOCATION\n";
        return true;
    }
}

const std::string&
Directives::getCgiExecutable(void) const
{
    return _cgiPass.cgiExec;
}

const std::vector<std::string>&
Directives::getCgiExtensions(void) const
{
    return _cgiPass.exts;
}
