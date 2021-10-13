#include "webserv/config-parser/ConfigParser.hpp"
#include "webserv/config-parser/ConfigBlock.hpp"
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

/*
 * The two static arrays defined below are used to construct
 * the _knownDirectives and _knownBlocks attributes of the
 * ConfigParser when its constructor is called.
 */

static const DirectiveCaracteristics knownDirectives[] = {
    { "root", NULL, ConfigBlock::BLOCK_SERVER | ConfigBlock::BLOCK_LOCATION },
    { "index", NULL, ConfigBlock::BLOCK_SERVER | ConfigBlock::BLOCK_LOCATION },
    { "method", NULL, ConfigBlock::BLOCK_SERVER },
    { "listen", NULL, ConfigBlock::BLOCK_SERVER },
    { "autoindex", NULL, ~0 },
    { "file_upload", NULL, ~0 },
    { "server_name", NULL, ConfigBlock::BLOCK_SERVER },
    { "client_body_maxsize", NULL, ConfigBlock::BLOCK_SERVER },
    { "default_error_file", NULL, ConfigBlock::BLOCK_GLOBAL },
};

static const BlockCaracteristics knownBlocks[] = {
    { "location", NULL, ConfigBlock::BLOCK_LOCATION },
    { "server", NULL, ConfigBlock::BLOCK_SERVER },
    { "GLOBAL", NULL, ConfigBlock::BLOCK_GLOBAL }
};

ConfigParser::ConfigParser(void)
{
    for (size_t i = 0;
         i != sizeof(knownDirectives) / sizeof(DirectiveCaracteristics);
         ++i) {
        std::pair<std::string, DirectiveCaracteristics> p(
          knownDirectives[i].name, knownDirectives[i]);
        _knownDirectives.insert(p);
    }

    for (size_t i = 0; i != sizeof(knownBlocks) / sizeof(BlockCaracteristics);
         ++i) {
        std::pair<std::string, BlockCaracteristics> p(knownBlocks[i].name,
                                                      knownBlocks[i]);
        _knownBlocks.insert(p);
    }
}

ConfigParser::ConfigParser(const ConfigParser& other)
{
    (void)other;
}

ConfigParser::~ConfigParser(void) {}

ConfigParser&
ConfigParser::operator=(const ConfigParser& rhs)
{
    if (this != &rhs) {
    }

    return *this;
}

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

ConfigBlock*
ConfigParser::parse(const std::vector<Lexer::Token>& tv)
{
    ConfigBlock *main = new ConfigBlock("GLOBAL", ConfigBlock::BLOCK_GLOBAL),
                *current = main, *tmp;
    std::pair<std::string, std::string> keyval;
    std::ostringstream oss;

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
                validateDirective(keyval, current);
                current->addDirective(keyval);
                keyval.first = "";
                keyval.second = "";
                break;
            case Lexer::BLOCK_START:
                tmp = new ConfigBlock(keyval.first,
                                      ConfigBlock::BLOCK_SERVER,
                                      keyval.second,
                                      current);
                current->_blocks.push_back(tmp);
                current = tmp;
                break;
            case Lexer::BLOCK_END:
                current = current->getParent();
                break;
            case Lexer::END_OF_FILE:
                break;
            default:
                break;
        }
    }

    return main;
}

ConfigBlock*
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

    ConfigBlock* block = parse(lex(data));

    printConfig(std::cout, block);

    return block;
}

/*
 * Print a block-based representation of the parsed config file, from the
 * original one. This is done recursively as each block has possibly many other
 * blocks as children, and so on.
 */

std::ostream&
ConfigParser::printConfig(std::ostream& os, ConfigBlock* main, size_t depth)
{
    std::cout << std::string(depth, '\t') << main->getName() << " "
              << main->getValue() << " {\n";

    for (DirectiveMap::const_iterator ite = main->getDirectiveMap().begin();
         ite != main->getDirectiveMap().end();
         ++ite) {
        os << std::string(depth + 1, '\t') << ite->first << " " << ite->second
           << "\n";
    }

    for (std::vector<ConfigBlock*>::const_iterator ite = main->_blocks.begin();
         ite != main->_blocks.end();
         ++ite) {
        printConfig(os, *ite, depth + 1);
    }

    std::cout << std::string(depth, '\t') << "}\n";

    return os;
}

void
ConfigParser::validateDirective(const Directive& direc, ConfigBlock* block)
{
    std::ostringstream oss;
    std::map<std::string, DirectiveCaracteristics>::const_iterator ite =
      _knownDirectives.find(direc.first);

    /* ensure that directive exists */
    if (ite == _knownDirectives.end()) {
        oss << "Found unsupported directive \"" << direc.first << "\"";
        throw ParserException(oss.str().c_str());
    }

    /* Ensure that the supported directive is valid in the block where it
     * appears */
    if (!(ite->second.validBlockContext & block->getType())) {
        oss << "Directive \"" << direc.first << "\" is not valid in block \""
            << block->getName() << "\"";
        throw ParserException(oss.str());
    }

    /* Run validator if there is one */
    if (ite->second.validator && !ite->second.validator(direc.second)) {
        oss << "Directive \"" << direc.first << "\" has an invalid value.";
        throw ParserException(oss.str());
    }
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