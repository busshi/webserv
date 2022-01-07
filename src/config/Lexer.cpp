#include "config/Lexer.hpp"
#include "utils/Formatter.hpp"
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
    return c == '{' || c == '}' || c == ';' || c == '#' || c == '\n';
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
                            "VALUE",   "EOF",         "SEMICOLON" };

    return types[type];
}

void
Lexer::skipSpace(void)
{
    while (ch() != '\n' && isspace(ch())) {
        movePos(1);
    }
}

void
Lexer::skipComment(void)
{
    while (ch() && ch() != '\n') {
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

Lexer::TokenType
Lexer::nextTokenType(void)
{
    size_t save_pos = _pos;
    TokenType type = UNKNOWN;

    while (ch() != '\n' && isspace(ch())) {
        ++_pos;
    }

    if (_pos >= _s.size()) {

        return END_OF_FILE;
    }

    if (!isreservedc(ch())) {
        return _tokenHistory.back().getType() == KEY ? VALUE : KEY;
    }

    switch (ch()) {
        case ';':
            type = SEMICOLON;
            break;
        case '{':
            type = BLOCK_START;
            break;
        case '}':
            type = BLOCK_END;
            break;
        case '\n':
            type = NEWLINE;
            break;
    }

    _pos = save_pos;

    return type;
}

Lexer::TokenType
Lexer::getLastRealTokenType(void)
{
    for (std::vector<Token>::const_reverse_iterator rite =
           _tokenHistory.rbegin();
         rite != _tokenHistory.rend();
         ++rite) {
        if (rite->getType() != NEWLINE) {
            return rite->getType();
        }
    }
    return UNKNOWN;
}

Lexer::Token
Lexer::makeToken(TokenType type, const std::string& value = "")
{
    _tokenHistory.push_back(Token(type, value));
    return Token(type, value);
}

Lexer::Lexer(const std::string& data)
  : _s(data)
  , _pos(0)
  , _blockDepth(0)
  , _lineNb(0)
  , _columnNb(0)
{
    _tokenHistory.push_back(Token(UNKNOWN, "unknown"));
}

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
Lexer::processOne(void)
{
    skipSpace();

    while (ch() == '#') {
        skipComment();
        skipSpace();
    }

    if (_pos >= _s.size()) {
        if (_blockDepth > 0) {
            throw LexerException(
              _lineNb, _columnNb, "Unclosed block, missing closing brace");
        }
        if (getLastRealTokenType() == KEY) {
            throw LexerException(
              _lineNb, _columnNb, "Unterminated key: missing value");
        }
        return makeToken(END_OF_FILE, "EOF");
    }

    /* tokenize key-value pair */

    if (!isreservedc(ch())) {
        if (getLastRealTokenType() != KEY) {
            return getKey();
        } else {
            return getValue();
        }
    }

    /* tokenize special char */

    char c = ch();
    movePos(1);

    TokenType type;

    switch (c) {
        case '\n':
            type = _tokenHistory.back().getType();
            if (type == VALUE && nextTokenType() != BLOCK_START) {
                throw LexerException(
                  _lineNb,
                  _columnNb,
                  "Unexpected line break, did you forget a ';' ?");
            }
            _columnNb = 0;
            ++_lineNb;
            return makeToken(NEWLINE, "NL");
        case ';':
            type = _tokenHistory.back().getType();
            if (type != VALUE) {
                throw LexerException(
                  _lineNb,
                  _columnNb,
                  "A semicolon is only valid after a directive's value");
            }
            while (ch() == ';') {
                movePos(1);
            }
            return makeToken(SEMICOLON, ";");
        case '{':
            type = getLastRealTokenType();
            if (type != VALUE && type != KEY) {
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
    return os << "\033[1;31mLexing Error\033[0m at line " << (_lineNb + 1)
              << ", column " << _columnNb << ": \033[1m" << _msg << "\033[0m";
}

// }}}
