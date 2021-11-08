#pragma once

#include <string>
#include <iostream>
#include <map>
#include <vector>
#include "fstream"
#include "sstream"
#include <sys/time.h>

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
		std::string	_getDate( void );

		std::map<std::string, std::string>	_headerParam;
		std::string	_response;
};
