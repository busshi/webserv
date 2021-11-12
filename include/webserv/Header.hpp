#pragma once

#include <string>
#include <iostream>
#include <map>
#include <vector>
#include "fstream"
#include "sstream"

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
		std::string	_getDate( time_t timestamp );
		std::string	_getLastModified( std::string path );
		std::string _replace(std::string in, std::string s1, std::string s2);
		void        _genErrorPage( std::string file, std::string code, std::string msg, std::string sentence );

		std::map<std::string, std::string>	_headerParam;
		std::string	_response;
};
