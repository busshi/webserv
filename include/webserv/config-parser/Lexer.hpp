#pragma once
#include <iostream>
#include <stdexcept>
#include <string>

class Lexer
{
  public:
    enum TokenType
    {
        UNKNOWN = 0,
        BLOCK_START,
        BLOCK_END,
        KEY,
        VALUE,
        END_OF_FILE,
        SEMICOLON,
    };

  public:
    // Lexer::Token {{{
    class Token
    {
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

    static std::string getTokenTypeAsString(TokenType type);

    class LexerException : public std::runtime_error
    {
        const char* _msg;
        size_t _lineNb, _columnNb;

      public:
        LexerException(size_t lineNb,
                       size_t columnNb,
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
    size_t _lineNb, _columnNb;
    TokenType _lastTokenType;

    void skipSpace(void);
    Token makeToken(TokenType type, const std::string& value);
    Token getKey(void);
    Token getValue(void);

    bool iskeyc(unsigned char c) const;
    bool isreservedc(unsigned char c) const;
    bool isvaluec(unsigned char c) const;

    /*
     * Returns current character element.
     * Shorthand for _s[_pos]
     */
    unsigned char ch(void) const;
    
    /*
     * Move the cursor of n characters.
     * Shortand for _pos += n
     * Adds n to _columnNb
     */
    void movePos(size_t n);

};

std::ostream&
operator<<(std::ostream& lhs, const Lexer::Token& rhs);