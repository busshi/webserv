#pragma once

#include <string>
#include <map>

class	Header {

	public:
		Header( void );
		~Header( void );

		Header( Header const & );

		Header &	operator=( Header const & );

		std::string	getResponse( void ) ;

		void		parseHeader( char buffer[], std::string );
		void		createResponse( void );

	private:
		void		_parseFirstLine( std::string, std::string );
		std::string _setParam( std::string ) ;
		std::string	_setContentType( std::string );

		std::map<std::string, std::string>	_headerMap;
		std::string	_response;
};
