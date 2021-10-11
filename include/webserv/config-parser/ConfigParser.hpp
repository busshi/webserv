#pragma once
#include "webserv/config-parser/Lexer.hpp"
#include <vector>

struct WebservConfig {};

class ConfigParser {
    std::vector<Lexer::Token> lex(const std::string& data);

  public:
    ConfigParser(void);
    ConfigParser(const ConfigParser& other);

    ~ConfigParser(void);

    ConfigParser& operator=(const ConfigParser& rhs);

    WebservConfig* loadConfig(const char* configPath);
};
