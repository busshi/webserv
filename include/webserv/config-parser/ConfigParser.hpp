#pragma once
#include <vector>
#include <map>
#include <string>
#include "webserv/config-parser/Lexer.hpp"

struct ConfigDirective {
  std::string key, value;
};

typedef std::map<std::string, ConfigDirective> DirectiveMap;

enum BlockType {
    BLOCK_GLOBAL = 0,
    BLOCK_LOCATION = 1 << 0,
    BLOCK_SERVER = 1 << 1
};

class ConfigBlock  {
  BlockType _type;
  DirectiveMap _directives;
  std::vector<ConfigBlock*> _blocks;

  public:
    ConfigBlock(BlockType type);
    ~ConfigBlock(void);

    BlockType getType(void) const;
    DirectiveMap& getDirectiveMap(void) const;
};

class ConfigParser {
    std::vector<Lexer::Token> lex(const std::string& data);
    ConfigBlock* parse(std::vector<Lexer::Token>);

  public:
    ConfigParser(void);
    ConfigParser(const ConfigParser& other);

    ~ConfigParser(void);

    ConfigParser& operator=(const ConfigParser& rhs);

    ConfigBlock* loadConfig(const char* configPath);
};
