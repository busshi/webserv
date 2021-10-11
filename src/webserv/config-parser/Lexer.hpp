#pragma once
# include <string>

class Lexer
{
	private:
		enum ContextFlag {
			INSIDE_BLOCK,
			EXPECT_VALUE,
		};

		std::string _s;
		std::string::size_type _pos;
		uint64_t _contextFlags;

	public:
		enum TokenType {
			UNKNOWN = -1,
			BLOCK_START,
			BLOCK_END,
			KEY,
			VALUE,
			SEMICOLON,
		};

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
};
