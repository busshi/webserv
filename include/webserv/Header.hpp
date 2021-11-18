#pragma once

#include <string>
#include <map>
#include <vector>
#include "config-parser/ConfigItem.hpp"

class	Header {

	public:
		Header( char buffer[] );
		~Header( void );

		Header( Header const & );

		Header &	operator=( Header const & );

		std::string	getResponse( void ) ;

		void		createResponse( ConfigItem * item );

	private:
		void			_parseFirstLine( std::string );
		std::string		_setParam( std::string ) ;
		std::string		_setContentType( std::string );
		std::string		_getDate( time_t timestamp );
		std::string		_getLastModified( std::string path );
		std::string		_replace( std::string in, std::string s1, std::string s2 );
		void       		_genErrorPage( std::string file, std::string code, std::string msg, std::string sentence );
		void		   	_noAutoIndexResponse( std::string path, std::stringstream & buf );
		void			_autoIndexResponse( std::string path, std::stringstream & buf );
		bool       		_isFolder( std::string path );
		std::string		_checkErrorPage(std::string defaultPage, std::string code, std::string errorMsg, std::string errorSentence );

		std::map<std::string, std::string>	_headerParam;
		std::string	_response;
};
