#pragma once

#include "Constants.hpp"
#include "config/ConfigItem.hpp"
#include "http/message.hpp"
#include "utils/string.hpp"
#include <string>
#include <vector>

class Directives
{

  public:
    Directives(void);
    ~Directives(void);

    Directives(const Directives&);
    Directives& operator=(const Directives&);

    void load(HTTP::Request* req, ConfigItem* block);

    std::string getRoot(void);
    std::string getPath(void);
    bool getAutoIndex(void);
    std::string getUploadStore(void);
    std::string getDefaultErrorFile(void);
    std::string getRewrite(void) const;
    std::string getRewriteLocation(void) const;
    bool dropsLocationPrefix(void) const;
    unsigned long long getUploadMaxFileSize(void);
    unsigned long long getBodyMaxSize(void);
    bool allowsCgi(void) const;
    bool allowsUpload(void) const;
    std::vector<std::string> getIndexes(void);
    std::vector<std::string> getForbiddenMethods(void);
    const std::string& getCgiExecutable(void) const;
    const std::vector<std::string>& getCgiExtensions(void) const;
    unsigned long long getRequestTimeout(void) const;

    bool _allowsCgi, _allowsUpload;

    void setPathWithIndex(void);

    bool haveLocation(std::string, std::string);

  private:
    std::string _root;
    std::string _path;
    bool _autoindex;
    std::string _uploadStore;
    std::string _defaultErrorFile;
    std::string _rewrite, _rewrite_location;
    bool _dropsLocationPrefix;
    unsigned long long _uploadMaxFileSize;
    unsigned long long _bodyMaxSize;
    std::vector<std::string> _indexes;
    std::vector<std::string> _forbiddenMethods;
    struct
    {
        std::string cgiExec;
        std::vector<std::string> exts;
    } _cgiPass;
};
