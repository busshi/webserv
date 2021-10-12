#include "webserv/config-parser/Lexer.hpp"
#include <cctype>
#include <iomanip>
#include <iostream>
#include <unistd.h>

bool
Lexer::iskeyc(unsigned char c) const
{
    return isalpha(c) || c == '_';
}

bool
Lexer::isreservedc(unsigned char c) const
{
    return c == '{' || c == '}' || c == ';' || c == '#';
}

bool
Lexer::isvaluec(unsigned char c) const
{
    return isprint(c) && !isreservedc(c);
}

unsigned char
Lexer::ch(void) const
{
    return _s[_pos];
}

void
Lexer::movePos(size_t n)
{
    _pos += n;
    _columnNb += n;
}

std::string
Lexer::getTokenTypeAsString(TokenType type)
{
    const char* types[] = { "UNKNOWN", "BLOCK_START", "BLOCK_END", "KEY",
                            "VALUE",   "EOF",         "SEMICOLON", "COMMENT" };

    return types[type];
}

void
Lexer::skipSpace(void)
{
    while (isspace(ch())) {
        if (ch() == '\n') {
            ++_lineNb;
            _columnNb = 0;
        }
        movePos(1);
    }
}

Lexer::Token
Lexer::getKey(void)
{
    size_t begPos = _pos;

    while (iskeyc(ch())) {
        movePos(1);
    }

    if (ch() != '{' && !isspace(ch())) {
        throw LexerException(
          _lineNb,
          _columnNb,
          "A directive name must only contain alphabetical characters");
    }

    return makeToken(KEY, _s.substr(begPos, _pos - begPos));
}

Lexer::Token
Lexer::getValue(void)
{
    size_t begPos = _pos;

    while (isvaluec(ch())) {
        movePos(1);
    }

    if (!ch()) {
        throw LexerException(_lineNb, _columnNb, "Unexpected end of file");
    }

    return makeToken(VALUE, _s.substr(begPos, _pos - begPos));
}

Lexer::Token
Lexer::makeToken(TokenType type, const std::string& value = "")
{
    _lastTokenType = type;
    return Token(type, value);
}

Lexer::Lexer(const std::string& data)
  : _s(data)
  , _pos(0)
  , _blockDepth(0)
  , _lineNb(0)
  , _columnNb(0)
  , _lastTokenType(Lexer::UNKNOWN)
{}

Lexer::Lexer(const Lexer& other)
{
    *this = other;
}

Lexer&
Lexer::operator=(const Lexer& rhs)
{
    if (this != &rhs) {
        _s = rhs._s;
        _pos = rhs._pos;
        _blockDepth = rhs._blockDepth;
    }

    return *this;
}

Lexer::~Lexer(void) {}

Lexer::Token
Lexer::next(void)
{
    skipSpace();

    if (_pos == _s.size()) {
        if (_blockDepth > 0) {
            throw LexerException(
              _lineNb, _columnNb, "Unclosed block, missing closing brace");
        }
        if (_lastTokenType == KEY) {
            throw LexerException(_lineNb, _columnNb, "Unexpected token");
        }
        return makeToken(END_OF_FILE, "EOF");
    }

    if (!isreservedc(ch())) {
        if (_lastTokenType != KEY) {
            return getKey();
        } else {
            return getValue();
        }
    }

    char c = ch();
    movePos(1);

    switch (c) {
        case ';':
            if (_lastTokenType != VALUE) {
                throw LexerException(
                  _lineNb,
                  _columnNb,
                  "A semicolon is only valid after a directive's value");
            }
            return makeToken(SEMICOLON, ";");
        case '{':
            if (_lastTokenType != KEY && _lastTokenType != VALUE) {
                throw LexerException(
                  _lineNb, _columnNb, "A block must have a name");
            }
            ++_blockDepth;
            return makeToken(BLOCK_START, "{");
        case '}':
            if (_blockDepth > 0) {
                --_blockDepth;
            } else {
                throw LexerException(
                  _lineNb, _columnNb, "Extraneous closing brace '}'");
            }
            return makeToken(BLOCK_END, "}");
        case '#':
            while (ch() && ch() != '\n') {
                movePos(1);
            }
            return makeToken(COMMENT, "#");
    }

    return Token(UNKNOWN);
}

// Lexer::Token {{{

Lexer::Token::Token(Lexer::TokenType type, const std::string& data)
  : _type(type)
  , _s(data)
{}

Lexer::Token::Token(const Lexer::Token& other)
{
    *this = other;
}

Lexer::Token::~Token(void) {}

Lexer::Token&
Lexer::Token::operator=(const Lexer::Token& rhs)
{
    if (this != &rhs) {
        _s = rhs._s;
        _type = rhs._type;
    }

    return *this;
}

const std::string&
Lexer::Token::getValue(void) const
{
    return _s;
}

Lexer::TokenType
Lexer::Token::getType(void) const
{
    return _type;
}

std::ostream&
operator<<(std::ostream& lhs, const Lexer::Token& rhs)
{
    return lhs << std::left << std::setw(15)
               << Lexer::getTokenTypeAsString(rhs.getType()) << rhs.getValue();
}

// }}}

// Lexer::LexerException {{{

Lexer::LexerException::LexerException(size_t lineNb,
                                      size_t columnNb,
                                      const char* msg)
  : std::runtime_error(msg)
  , _msg(msg)
  , _lineNb(lineNb)
  , _columnNb(columnNb)
{}

Lexer::LexerException::LexerException(const Lexer::LexerException& other)
  : std::runtime_error(other._msg)
{
    *this = other;
}

Lexer::LexerException&
Lexer::LexerException::operator=(const Lexer::LexerException& rhs)
{
    if (this != &rhs) {
        _columnNb = rhs._columnNb;
        _lineNb = rhs._lineNb;
        _msg = rhs._msg;
    }

    return *this;
}

Lexer::LexerException::~LexerException(void) throw() {}

size_t
Lexer::LexerException::getLineNumber(void) const
{
    return _lineNb;
}

size_t
Lexer::LexerException::getColumnNumber(void) const
{
    return _columnNb;
}

std::ostream&
Lexer::LexerException::printFormatted(std::ostream& os)
{
    return os << "\033[1;31mLexing Error\033[0m at line " << _lineNb
              << ", column " << _columnNb << ": \033[1m" << _msg << "\033[0m";
}

// }}}
