#pragma once
#include "webserv/config-parser/ConfigItem.hpp"
#include "webserv/config-parser/Lexer.hpp"
#include <map>
#include <stdexcept>
#include <stdint.h>
#include <string>
#include <vector>

struct ConfigItemCaracteristics
{
    std::string name;
    bool (*validator)(const std::string& value, std::string& errorMsg);
    uint8_t validBlockContext;
    BlockType blockType;
};

std::ostream&
operator<<(std::ostream& os, ConfigItem* block);

class ConfigParser
{
    std::map<std::string, ConfigItemCaracteristics> _knownConfigItems;

    std::vector<Lexer::Token> lex(const std::string& data);
    ConfigItem* parse(const std::vector<Lexer::Token>& tv);

    /*
    ** Allocate and initialize a new ConfigItem given a key-value pair.
    **
    ** This function also validates the key-value pair which is sent, making
    sure that it refers to a valid configuration atom.
    **
    ** `contextItem` refers to the context, also known as block, inside which
    the currently processed atom has been found.
    **
    ** The following things are checked:
    ** - Whether or not the key refers to a valid configuration atom (all the
    available atoms are stored in _knownConfigItems).
    ** - Whether or not the configuration atom is valid in the current context
    (also called block).
    **
    ** In case one of these checks returns false, a relevant `ParsingException`
    is thrown.
    ** If every check succeeds, a pointer to the new configuration item is
    returned.
    */

    ConfigItem* makeConfigItem(std::pair<std::string, std::string> keyval,
                               ConfigItem* contextItem);

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

    ConfigItem* loadConfig(const char* configPath);

    std::ostream& printConfig(std::ostream& os,
                              ConfigItem* main,
                              size_t depth = 0);
};

/* Parse utils */

struct ListenData
{
    std::string v4; /* [IPv4:]port */
    uint16_t port;
    bool isDefault;
};

ListenData
parseListen(const std::string& listenDirective);

unsigned long long
parseSize(const std::string& size);
