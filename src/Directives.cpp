#include "Directives.hpp"
#include "config/ConfigItem.hpp"
#include "core.hpp"
#include "http/Exception.hpp"
#include "http/message.hpp"
#include "http/status.hpp"
#include "utils/Logger.hpp"

Directives::Directives(void)
  : _allowsCgi(false)
  , _allowsUpload(false)
{}

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
        _uploadStore = rhs._uploadStore;
        _defaultErrorFile = rhs._defaultErrorFile;
        _uploadMaxFileSize = rhs._uploadMaxFileSize;
        _bodyMaxSize = rhs._bodyMaxSize;
        _indexes = rhs._indexes;
        _forbiddenMethods = rhs._forbiddenMethods;
    }

    return *this;
}

std::string
Directives::getRewrite() const
{
    return _rewrite;
}

bool
Directives::allowsCgi(void) const
{
    return _allowsCgi;
}

bool
Directives::allowsUpload(void) const
{
    return _allowsUpload;
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

bool
Directives::getAutoIndex(void)
{
    return _autoindex;
}

std::string
Directives::getUploadStore(void)
{
    return _uploadStore;
}

std::string
Directives::getDefaultErrorFile(void)
{
    return _defaultErrorFile;
}

unsigned long long
Directives::getUploadMaxFileSize(void)
{
    return _uploadMaxFileSize;
}

unsigned long long
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
Directives::getForbiddenMethods(void)
{
    return _forbiddenMethods;
}

void
Directives::load(HTTP::Request* req, ConfigItem* item)
{

    ConfigItem* root = item->findNearest("root");

    if (!root) {
        throw HTTP::Exception(
          req,
          HTTP::INTERNAL_SERVER_ERROR,
          "No root directive in configuration for that block");
    }

    _root = trimTrailing(root->getValue(), "/");
    _path = _root;
    _path += req->getLocation();

    ConfigItem* rewrite = item->findAtomInBlock("rewrite");

    if (rewrite) {
        _rewrite = rewrite->getValue();
    }

    ConfigItem* autoindex = item->findNearest("autoindex");

    _autoindex = autoindex && autoindex->getValue() == "on";

    ConfigItem* index = item->findNearest("index");
    if (index)
        _indexes = split(index->getValue());

    ConfigItem* methods = item->findNearest("forbidden_methods");
    if (methods)
        _forbiddenMethods = split(methods->getValue());

    ConfigItem* uploadStore = item->findAtomInBlock("upload_store");
    if (uploadStore) {
        _allowsUpload = true;
        _uploadStore = uploadStore->getValue();
    }

    ConfigItem* uploadMaxSize = item->findNearest("upload_max_file_size");
    _uploadMaxFileSize = parseSize(uploadMaxSize ? uploadMaxSize->getValue()
                                                 : DFLT_MAX_UPLOAD_FILE_SIZE);

    ConfigItem* bodyMaxSize = item->findNearest("client_body_max_size");
    _bodyMaxSize =
      parseSize(bodyMaxSize ? bodyMaxSize->getValue() : DFLT_MAX_BODY_SIZE);

    // search ONLY in the current block
    ConfigItem* cgiPass = item->findAtomInBlock("cgi_pass");
    if (cgiPass) {
        _allowsCgi = true;
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
