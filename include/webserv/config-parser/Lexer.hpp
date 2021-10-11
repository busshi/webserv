#pragma once
#include <stdexcept>
#include <string>

class Lexer {
  public:
    enum TokenType {
        UNKNOWN = -1,
        BLOCK_START,
        BLOCK_END,
        KEY,
        VALUE,
        SEMICOLON,
        END_OF_FILE,
    };

  public:
    // Lexer::Token {{{
    class Token {
        TokenType _type;
        std::string _s;

      public:
        Token(TokenType type, const std::string& data = "");
        Token(const Token& other);

        ~Token(void);

        Token& operator=(const Token& rhs);

        const std::string& getValue(void) const;
        TokenType getType(void) const;
    };
    // }}}

    Lexer(const std::string& data);
    Lexer(const Lexer& other);

    ~Lexer(void);

    Token next(void);

    Lexer& operator=(const Lexer& other);

    class LexerException : public std::runtime_error {
        const char* _msg;
        size_t _lineNb, _columnNb;

      public:
        LexerException(size_t lineNb, size_t columnNb,
                       const char* msg = "Unknown error");
        LexerException(const LexerException& other);

        ~LexerException(void) throw();

        LexerException& operator=(const LexerException& rhs);

        size_t getLineNumber(void) const;
        size_t getColumnNumber(void) const;

        std::ostream& printFormatted(std::ostream& os);
    };

  private:
    std::string _s;
    std::string::size_type _pos;
    size_t _blockDepth;
    size_t _lineNb;
    TokenType _lastTokenType;

    void skipSpace(void);
    Token makeToken(TokenType type, const std::string& value);
};
