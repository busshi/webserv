#pragma once

#include <string>

class	Header {

	public:
		Header( void );
		~Header( void );

		Header( Header const & );

		Header &	operator=( Header const & );

		std::string	getResponse( void ) ;

		void		parseHeader( char buffer[] );
		void		createResponse( void );
		void		display( void );

	private:
		std::string	_method;
		std::string	_path;
		std::string	_response;
		std::string	_statusCode;
		std::string	_contentType;
		std::string	_contentLen;
};
