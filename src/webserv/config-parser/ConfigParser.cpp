#include <fstream>
#include <iomanip>
#include "webserv/config-parser/ConfigParser.hpp"

ConfigParser::ConfigParser(void) {}

ConfigParser::ConfigParser(const ConfigParser& other) { (void)other; }

ConfigParser::~ConfigParser(void) {}

ConfigParser& ConfigParser::operator=(const ConfigParser& rhs) {
    if (this != &rhs) {
    }

    return *this;
}

std::vector<Lexer::Token> ConfigParser::lex(const std::string& data) {
    std::vector<Lexer::Token> v;
    Lexer lexer(data);

    do {
        v.push_back(lexer.next());
    } while (v.back().getType() != Lexer::END_OF_FILE);

    return v;
}

ConfigBlock* ConfigParser::parse(const std::vector<Lexer::Token>& tv)
{
    (void)tv;
    ConfigBlock* main = new ConfigBlock(BLOCK_GLOBAL), *current = main, *tmp;
    std::pair<std::string, std::string> keyval;

    for (std::vector<Lexer::Token>::const_iterator ite = tv.begin(); ite != tv.end(); ++ite) {
        switch (ite->getType()) {
            case Lexer::KEY:
                keyval.first = ite->getValue();
                break ;
            case Lexer::VALUE:
                keyval.second = ite->getValue();
                break ;
            case Lexer::SEMICOLON:
                current->getDirectiveMap().insert(keyval);
                break ;
            case Lexer::BLOCK_START:
                tmp = new ConfigBlock(BLOCK_SERVER, current);
                current->_blocks.push_back(tmp);
                current = tmp;
                break ;
            case Lexer::BLOCK_END:
                current = current->getParent();
                break ;
            case Lexer::END_OF_FILE:
                break ;
            default:
                std::cerr << "Not supported yet :o\n";
        }
    }

    return main;
}

ConfigBlock* ConfigParser::loadConfig(const char* configPath) {
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

std::ostream& operator<<(std::ostream& os, ConfigBlock* main)
{
    for (DirectiveMap::iterator ite = main->getDirectiveMap().begin(); ite != main->getDirectiveMap().end(); ++ite) {
        os << "\t" << ite->first << " " << ite->second << "\n";
    }

    return os;
}

/*
 * Print a block-based representation of the parsed config file, from the original one.
 * This is done recursively as each block has possibly many other blocks as children, and so on.
 */

std::ostream& ConfigParser::printConfig(std::ostream& os, ConfigBlock* main, size_t depth)
{
    std::cout << std::string(depth, '\t') << "{\n";

    for (DirectiveMap::iterator ite = main->getDirectiveMap().begin();
            ite != main->getDirectiveMap().end(); ++ite) {
        os << std::string(depth + 1, '\t') << ite->first << " " << ite->second << "\n";
    }

    for (std::vector<ConfigBlock*>::iterator it = main->_blocks.begin(); it != main->_blocks.end(); ++it) {
        printConfig(os, *it, depth + 1);
    }

    std::cout << std::string(depth, '\t') << "}\n";

    return os;
}

// ConfigBlocks {{{

ConfigBlock::ConfigBlock(BlockType type, ConfigBlock* parent): _type(type), _parent(parent) {}

BlockType ConfigBlock::getType(void) const { return _type; }
DirectiveMap& ConfigBlock::getDirectiveMap(void) { return _directives; }
ConfigBlock* ConfigBlock::getParent(void) const { return _parent; }

ConfigBlock::~ConfigBlock(void)
{
    for (std::vector<ConfigBlock*>::const_iterator ite = _blocks.begin(); ite != _blocks.end(); ++ite) {
        delete *ite;
    }
}

// }}}
