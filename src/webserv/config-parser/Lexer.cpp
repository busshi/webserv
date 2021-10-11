#include "webserv/config-parser/Lexer.hpp"

#include <cctype>
#include <iostream>

void Lexer::skipSpace(void) {
    while (isspace(_s[_pos])) {
        if (_s[_pos] == '\n') {
            ++_lineNb;
        }
        ++_pos;
    }
}

Lexer::Token Lexer::makeToken(TokenType type, const std::string& value = "") {
    _lastTokenType = type;
    ++_pos;
    return Token(type, value);
}

Lexer::Lexer(const std::string& data)
    : _s(data), _pos(0), _blockDepth(0), _lineNb(0),
      _lastTokenType(Lexer::UNKNOWN) {}

Lexer::Lexer(const Lexer& other) { *this = other; }

Lexer& Lexer::operator=(const Lexer& rhs) {
    if (this != &rhs) {
        _s = rhs._s;
        _pos = rhs._pos;
        _blockDepth = rhs._blockDepth;
    }

    return *this;
}

Lexer::~Lexer(void) {}

Lexer::Token Lexer::next(void) {
    skipSpace();

    if (_pos == _s.size()) {
        if (_blockDepth > 0) {
            throw LexerException(_lineNb, 0,
                                 "Unclosed block, missing closing brace");
        }
        return makeToken(END_OF_FILE, "EOF");
    }

    switch (_s[_pos]) {
    case '{':
        ++_blockDepth;
        return makeToken(BLOCK_START, "{");
    case '}':
        if (_blockDepth > 0) {
            --_blockDepth;
        } else {
            throw LexerException(_lineNb, 0, "Extraneous closing brace '}'");
        }
        return makeToken(BLOCK_END, "}");
    }

    if (!isalpha(_s[_pos])) {
        std::cerr
            << "Unexpected token " << _s[_pos]
            << ": directive names only contain alphabetical characters.\n";
        return makeToken(UNKNOWN);
    }

    return Token(UNKNOWN);
}

// Lexer::Token {{{

Lexer::Token::Token(Lexer::TokenType type, const std::string& data)
    : _type(type), _s(data) {}

Lexer::Token::Token(const Lexer::Token& other) { *this = other; }

Lexer::Token::~Token(void) {}

Lexer::Token& Lexer::Token::operator=(const Lexer::Token& rhs) {
    if (this != &rhs) {
        _s = rhs._s;
        _type = rhs._type;
    }

    return *this;
}

const std::string& Lexer::Token::getValue(void) const { return _s; }

Lexer::TokenType Lexer::Token::getType(void) const { return _type; }

// }}}

// Lexer::LexerException {{{

Lexer::LexerException::LexerException(size_t lineNb, size_t columnNb,
                                      const char* msg)
    : std::runtime_error(msg), _msg(msg), _lineNb(lineNb), _columnNb(columnNb) {
}

Lexer::LexerException::LexerException(const Lexer::LexerException& other)
    : std::runtime_error(other._msg) {
    *this = other;
}

Lexer::LexerException&
Lexer::LexerException::operator=(const Lexer::LexerException& rhs) {
    if (this != &rhs) {
        _columnNb = rhs._columnNb;
        _lineNb = rhs._lineNb;
        _msg = rhs._msg;
    }

    return *this;
}

Lexer::LexerException::~LexerException(void) throw() {}

size_t Lexer::LexerException::getLineNumber(void) const { return _lineNb; }

size_t Lexer::LexerException::getColumnNumber(void) const { return _columnNb; }

std::ostream& Lexer::LexerException::printFormatted(std::ostream& os) {
    return os << "\033[1;31mLexing Error\033[0m at line " << _lineNb
              << ", column " << _columnNb << ": \033[1m" << _msg << "\033[0m";
}

// }}}

