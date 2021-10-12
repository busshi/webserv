#include <fstream>
#include <iomanip>
#include <sstream>
#include "webserv/config-parser/ConfigParser.hpp"

/*
* The two static arrays defined below are used to construct
* the _knownDirectives and _knownBlocks attributes of the
* ConfigParser when its constructor is called.
*/

static const DirectiveCaracteristics knownDirectives[] = {
    { "root", NULL, BLOCK_SERVER |  BLOCK_LOCATION },
    { "index", NULL, BLOCK_SERVER | BLOCK_LOCATION },
    { "method", NULL, BLOCK_SERVER },
    { "listen", NULL, BLOCK_SERVER },
    { "autoindex", NULL, ~0 },
    { "file_upload", NULL, ~0 },
    { "server_name", NULL, BLOCK_SERVER },
    { "client_body_maxsize", NULL, BLOCK_SERVER },
    { "default_error_file", NULL,  BLOCK_GLOBAL },
};

static const BlockCaracteristics knownBlocks[] = {
    { "location", NULL, BLOCK_LOCATION },
    { "server", NULL, BLOCK_SERVER },
    { "GLOBAL", NULL, BLOCK_GLOBAL }
};

ConfigParser::ConfigParser(void)
{
   for (size_t i = 0; i != sizeof(knownDirectives) / sizeof (DirectiveCaracteristics); ++i) {
       std::pair<std::string, DirectiveCaracteristics> p(knownDirectives[i].name, knownDirectives[i]);
       _knownDirectives.insert(p);
   }
   
    for (size_t i = 0; i != sizeof(knownBlocks) / sizeof (BlockCaracteristics); ++i) {
       std::pair<std::string, BlockCaracteristics> p(knownBlocks[i].name, knownBlocks[i]);
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
    (void)tv;
    ConfigBlock *main = new ConfigBlock("GLOBAL", BLOCK_GLOBAL), *current = main, *tmp;
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
                if (!isValidDirective(keyval.first)) {
                    oss << "Directive \"" << keyval.first << "\" does not exist";
                    throw std::runtime_error(oss.str());
                }
                if (!isValidDirectiveInBlock(keyval.first, current)) {
                    oss << "Directive \"" << keyval.first << "\" is not allowed in block \"" << current->getName() << "\"";
                    throw std::runtime_error(oss.str());
                }
                current->addDirective(keyval);
                keyval.first = "";
                keyval.second = "";
                break;
            case Lexer::BLOCK_START:
                tmp = new ConfigBlock(keyval.first, BLOCK_GLOBAL, keyval.second, current);
                current->_blocks.push_back(tmp);
                current = tmp;
                break;
            case Lexer::BLOCK_END:
                current = current->getParent();
                break;
            case Lexer::END_OF_FILE:
                break;
            default:
                std::cerr << "Not supported yet :o\n";
        }
    }

    return main;
}

bool ConfigParser::isValidDirective(const std::string& name) const
{
    return (_knownDirectives.find(name) != _knownDirectives.end());
}

bool ConfigParser::isValidDirectiveInBlock(const std::string& name, ConfigBlock* block)
{
    std::map<std::string, DirectiveCaracteristics>::const_iterator ite = _knownDirectives.find(name);

    return (ite != _knownDirectives.end() && ite->second.validBlockContext & block->getType());
}

void
ConfigBlock::addDirective(const DirectiveMap::value_type& value)
{
    _directives.insert(value);
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

// ConfigBlocks {{{

ConfigBlock::ConfigBlock(std::string name,
                        BlockType type,
                         std::string value,
                         ConfigBlock* parent)
  : _parent(parent)
  , _name(name)
  , _value(value)
  , _type(type)
{}

const DirectiveMap&
ConfigBlock::getDirectiveMap(void) const
{
    return _directives;
}

ConfigBlock*
ConfigBlock::getParent(void) const
{
    return _parent;
}

const std::string&
ConfigBlock::getName(void) const
{
    return _name;
}

const std::string&
ConfigBlock::getValue(void) const
{
    return _value;
}

BlockType ConfigBlock::getType(void) const
{
    return _type;
}

ConfigBlock::~ConfigBlock(void)
{
    for (std::vector<ConfigBlock*>::const_iterator ite = _blocks.begin();
         ite != _blocks.end();
         ++ite) {
        delete *ite;
    }
}

// }}}
