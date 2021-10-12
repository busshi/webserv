#pragma once
#include "webserv/config-parser/ConfigBlock.hpp"
#include "webserv/config-parser/Lexer.hpp"
#include <map>
#include <stdexcept>
#include <stdint.h>
#include <string>
#include <vector>

typedef std::pair<std::string, std::string> Directive;

struct DirectiveCaracteristics
{
    std::string name;
    bool (*validator)(const std::string& value);
    uint8_t validBlockContext;
};

struct BlockCaracteristics
{
    std::string name;
    bool (*validator)(const std::string);
    ConfigBlock::BlockType type;
};

typedef std::map<std::string, std::string> DirectiveMap;

std::ostream&
operator<<(std::ostream& os, ConfigBlock* block);

class ConfigParser
{
    std::map<std::string, BlockCaracteristics> _knownBlocks;
    std::map<std::string, DirectiveCaracteristics> _knownDirectives;

    std::vector<Lexer::Token> lex(const std::string& data);
    ConfigBlock* parse(const std::vector<Lexer::Token>& tv);

    bool isValidDirective(const std::string& name) const;
    bool isValidDirectiveInBlock(const std::string& name, ConfigBlock* block);

    /*
     * Ensure, given a directive name, that it is supported and valid in the
     * current block. If any, calls the associated validator method which
     * determines if the value of the directive is valid. In case of failure,
     * a ParserException is thrown.
     */
    void validateDirective(const Directive& direc, ConfigBlock* block);

  public:
    class ParserException : public std::runtime_error
    {
      public:
        explicit ParserException(const std::string& msg);

        std::ostream& printFormatted(std::ostream& os);
    };

    ConfigParser(void);
    ConfigParser(const ConfigParser& other);

    ~ConfigParser(void);

    ConfigParser& operator=(const ConfigParser& rhs);

    ConfigBlock* loadConfig(const char* configPath);

    std::ostream& printConfig(std::ostream& os,
                              ConfigBlock* main,
                              size_t depth = 0);
};
