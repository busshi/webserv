#pragma once

#include "Constants.hpp"
#include "config/ConfigItem.hpp"
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

    void getConfig(ConfigItem*, std::string);

    std::string getRoot(void);
    std::string getPath(void);
    std::string getAutoIndex(void);
    std::string getUploadStore(void);
    std::string getDefaultErrorFile(void);
    std::string getUploadMaxFileSize(void);
    std::string getBodyMaxSize(void);
    std::vector<std::string> getIndexes(void);
    std::vector<std::string> getMethods(void);
    const std::string& getCgiExecutable(void) const;
    const std::vector<std::string>& getCgiExtensions(void) const;

    void setPathWithIndex(void);

    bool haveLocation(std::string, std::string);

  private:
    std::string _root;
    std::string _path;
    std::string _autoindex;
    std::string _uploadStore;
    std::string _defaultErrorFile;
    std::string _uploadMaxFileSize;
    std::string _bodyMaxSize;
    std::vector<std::string> _indexes;
    std::vector<std::string> _methods;
    struct
    {
        std::string cgiExec;
        std::vector<std::string> exts;
    } _cgiPass;
};