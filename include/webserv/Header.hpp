#pragma once

#include <string>
#include <map>
#include <vector>

class	Header {

	public:
		Header( void );
		~Header( void );

		Header( Header const & );

		Header &	operator=( Header const & );

		std::string	getResponse( void ) ;

		void		parseHeader( char buffer[], std::string );
		void		createResponse( std::string autoindex, std::vector<std::string> indexes );

	private:
		void				_parseFirstLine( std::string, std::string );
		std::string			_setParam( std::string ) ;
		std::string			_setContentType( std::string );
		std::string			_getDate( time_t timestamp );
		std::string			_getLastModified( std::string path );
		std::string			_replace( std::string in, std::string s1, std::string s2 );
		std::string			_getIndex( std::string path, std::vector<std::string> indexes );
		void        		_genErrorPage( std::string file, std::string code, std::string msg, std::string sentence );
		void			   	_noAutoIndexResponse( std::string path, std::stringstream & buf, std::string autoindex );
		void				_autoIndexResponse( std::string path, std::stringstream & buf, std::string autoindex );

		std::map<std::string, std::string>	_headerParam;
		std::string	_response;
};
