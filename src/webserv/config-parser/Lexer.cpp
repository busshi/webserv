#include <iostream>
#include <cctype>
#include "webserv/config-parser/Lexer.hpp"

Lexer::Lexer(const std::string& data): _s(data), _pos(0), _blockDepth(0) {}

Lexer::Lexer(const Lexer& other)
{
	*this = other;
}

Lexer& Lexer::operator=(const Lexer& rhs)
{
	if (this != &rhs) {
		_s = rhs._s;
		_pos = rhs._pos;
		_blockDepth = rhs._blockDepth;
	}

	return *this;
}

Lexer::~Lexer(void)
{
}

Lexer::Token Lexer::next(void)
{
	if (_pos == _s.size()) {
		if (_blockDepth > 0) {
			std::cerr << "Unclosed block\n";
		}
		return Token(END_OF_FILE, "EOF");
	}

	if (isspace(_s[_pos])) {
		while (isspace(_s[_pos])) {
			++_pos;
		}
	}

	switch (_s[_pos++]) {
		case '{':
			++_blockDepth;
			return Token(BLOCK_START, "{");
		case '}':
			if (_blockDepth > 0) {
				--_blockDepth;
			} else {
				std::cerr << "Unexpected }\n";
			}
			return Token(BLOCK_END, "}");
	}
	
	return Token(UNKNOWN);
}

// Lexer::Token {{{

Lexer::Token::Token(Lexer::TokenType type, const std::string& data): _type(type), _s(data) {}

Lexer::Token::Token(const Lexer::Token& other)
{
	*this = other;
}

Lexer::Token::~Token(void)
{
}

Lexer::Token& Lexer::Token::operator=(const Lexer::Token& rhs)
{
	if (this != &rhs) {
		_s = rhs._s;
		_type = rhs._type;
	}

	return *this;
}

const std::string& Lexer::Token::getValue(void) const { return _s; }

Lexer::TokenType Lexer::Token::getType(void) const { return _type; }

// }}}
