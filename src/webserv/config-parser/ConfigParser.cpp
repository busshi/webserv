#include "webserv/config-parser/ConfigParser.hpp"
#include "utils/Formatter.hpp"
#include "utils/string.hpp"
#include "webserv/config-parser/validator.hpp"
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

static const ConfigItemCaracteristics knownConfigItems[] = {
    /* BLOCKS */
    { "location", NULL, BLOCK_SERVER, BLOCK_LOCATION },
    { "server", NULL, BLOCK_GLOBAL, BLOCK_SERVER },

    /* NON-BLOCK */
    { "root", NULL, BLOCK_SERVER | BLOCK_LOCATION, NOT_A_BLOCK },
    { "index", NULL, BLOCK_SERVER | BLOCK_LOCATION, NOT_A_BLOCK },
    { "method", NULL, BLOCK_SERVER, NOT_A_BLOCK },
    { "listen", NULL, BLOCK_SERVER, NOT_A_BLOCK },
    { "autoindex", validateAutoindex, ~0, NOT_A_BLOCK },
    { "file_upload", NULL, ~0, NOT_A_BLOCK },
    { "server_name", NULL, BLOCK_SERVER, NOT_A_BLOCK },
    { "client_body_maxsize", NULL, BLOCK_SERVER, NOT_A_BLOCK },
    { "default_error_file", NULL, BLOCK_GLOBAL, NOT_A_BLOCK },
};

ConfigParser::ConfigParser(void)
{
    for (size_t i = 0;
         i != sizeof(knownConfigItems) / sizeof(ConfigItemCaracteristics);
         ++i) {
        std::pair<std::string, ConfigItemCaracteristics> p(
          knownConfigItems[i].name, knownConfigItems[i]);
        _knownConfigItems.insert(p);
    }
}

ConfigParser::~ConfigParser() {}

std::vector<Lexer::Token>
ConfigParser::lex(const std::string& data)
{
    std::vector<Lexer::Token> v;
    Lexer lexer(data);

    do {
        v.push_back(lexer.next());
    } while (v.back().getType() != Lexer::END_OF_FILE);

    return v;
}

ConfigItem*
ConfigParser::parse(const std::vector<Lexer::Token>& tv)
{
    ConfigItem *main = new ConfigItem, *current = main, *tmp = 0;
    main->name = "GLOBAL";
    main->blockType = BLOCK_GLOBAL;

    std::pair<std::string, std::string> keyval;

    for (std::vector<Lexer::Token>::const_iterator ite = tv.begin();
         ite != tv.end();
         ++ite) {
        switch (ite->getType()) {
            case Lexer::KEY:
                keyval.first = ite->getValue();
                break;
            case Lexer::VALUE:
                keyval.second = ite->getValue();
                break;
            case Lexer::SEMICOLON:
                current->children.push_back(makeConfigItem(keyval, current));
                keyval.first = "";
                keyval.second = "";
                break;
            case Lexer::BLOCK_START:
                // TODO: add isBlock validation
                tmp = makeConfigItem(keyval, current);
                tmp->parent = current;
                current->children.push_back(tmp);
                current = tmp;
                break;
            case Lexer::BLOCK_END:
                current = current->parent;
                break;
            case Lexer::END_OF_FILE:
                break;
            default:
                break;
        }
    }

    return main;
}

ConfigItem*
ConfigParser::loadConfig(const char* configPath)
{
    std::ifstream ifs(configPath);
    std::string line, data;

    if (!ifs) {
        throw std::runtime_error("Could not open config file");
    }

    while (std::getline(ifs, line)) {
        data += line + '\n';
    }

    ConfigItem* main = parse(lex(data));

    printConfig(std::cout, main);

    return main;
}

/*
 * Print a block-based representation of the parsed config file, from the
 * original one. This is done recursively as each block has possibly many other
 * blocks as children, and so on.
 */

std::ostream&
ConfigParser::printConfig(std::ostream& os, ConfigItem* main, size_t depth)
{
    std::cout << std::string(depth, '\t') << main->name << " " << main->value
              << (main->blockType != NOT_A_BLOCK ? " {" : "") << "\n";

    for (std::vector<ConfigItem*>::const_iterator ite = main->children.begin();
         ite != main->children.end();
         ++ite) {
        printConfig(os, *ite, depth + 1);
    }

    if (main->blockType != NOT_A_BLOCK) {
        std::cout << std::string(depth, '\t') << "}\n";
    }

    return os;
}

ConfigItem*
ConfigParser::makeConfigItem(std::pair<std::string, std::string> keyval,
                             ConfigItem* contextItem)
{
    std::string errorMsg;
    std::map<std::string, ConfigItemCaracteristics>::const_iterator ite =
      _knownConfigItems.find(keyval.first);

    if (ite == _knownConfigItems.end()) {
        Formatter() << "\"" << keyval.first
                    << "\" does not refer to a valid configuration property." >>
          errorMsg;
        throw ParserException(errorMsg);
    }

    if (!(ite->second.validBlockContext &
          (!contextItem ? BLOCK_GLOBAL : contextItem->blockType))) {
        Formatter() << "Name \"" << keyval.first
                    << "\" is not allowed in context \"" << contextItem->name
                    << "\"" >>
          errorMsg;
        throw ParserException(errorMsg);
    }

    if (ite->second.blockType == NOT_A_BLOCK &&
        std::find_if(contextItem->children.begin(),
                     contextItem->children.end(),
                     FindConfigItemPredicate(keyval.first)) !=
          contextItem->children.end()) {
        Formatter() << "Name \"" << keyval.first
                    << "\" duplicated in context \"" << contextItem->name
                    << "\"" >>
          errorMsg;
        throw ParserException(errorMsg);
    }

    if (ite->second.validator &&
        !ite->second.validator(trim(keyval.second), errorMsg)) {
        throw ParserException(errorMsg);
    }

    ConfigItem* item = new ConfigItem;
    item->name = keyval.first;
    item->value = keyval.second;
    item->blockType = ite->second.blockType;

    return item;
}

ConfigParser::ParserException::ParserException(const std::string& msg)
  : std::runtime_error(msg)
{}

std::ostream&
ConfigParser::ParserException::printFormatted(std::ostream& os)
{
    return os << "\033[1;31mParser error\033[0m: \033[1m" << what()
              << "\033[0m\n";
}