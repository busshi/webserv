#pragma once
#include "webserv/config-parser/Lexer.hpp"
#include <stdint.h>
#include <map>
#include <string>
#include <vector>

enum BlockType
{
    BLOCK_GLOBAL = 1 << 0,
    BLOCK_LOCATION = 1 << 1,
    BLOCK_SERVER = 1 << 2
};

struct DirectiveCaracteristics {
  std::string name;
  bool (*validator)(const std::string& value);
  uint8_t validBlockContext;
};

struct BlockCaracteristics {
  std::string name;
  bool (*validator)(const std::string);
  BlockType type;
};

typedef std::map<std::string, std::string> DirectiveMap;

class ConfigBlock
{
    DirectiveMap _directives;
    ConfigBlock* _parent;
    std::string _name, _value;
    BlockType _type;

  public:
    std::vector<ConfigBlock*> _blocks;
    ConfigBlock(std::string name, BlockType type, std::string value = "",  ConfigBlock* parent = 0);
    ~ConfigBlock(void);

    void addDirective(const DirectiveMap::value_type& value);

    const DirectiveMap& getDirectiveMap(void) const;
    ConfigBlock* getParent(void) const;
    const std::string& getName(void) const;
    const std::string& getValue(void) const;
    BlockType getType(void) const;
};

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

  public:
    ConfigParser(void);
    ConfigParser(const ConfigParser& other);

    ~ConfigParser(void);

    ConfigParser& operator=(const ConfigParser& rhs);
    
    ConfigBlock* loadConfig(const char* configPath);

    std::ostream& printConfig(std::ostream& os,
                              ConfigBlock* main,
                              size_t depth = 0);
};
