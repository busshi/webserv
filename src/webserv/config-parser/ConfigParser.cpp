#include "webserv/config-parser/ConfigParser.hpp"
#include <fstream>

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
        std::cout << v.back() << "\n";
    } while (v.back().getType() != Lexer::END_OF_FILE);

    return v;
}

WebservConfig* ConfigParser::loadConfig(const char* configPath) {
    std::ifstream ifs(configPath);
    std::string line, data;

    if (!ifs) {
        throw std::runtime_error("Could not open config file");
    }

    while (std::getline(ifs, line)) {
        data += line + '\n';
    }

    lex(data);

    return new WebservConfig();
}
