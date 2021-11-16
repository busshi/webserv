#pragma once

#include <string>
#include <map>
#include <vector>
#include "config-parser/ConfigItem.hpp"

class	Header {

	public:
		Header( void );
		~Header( void );

		Header( Header const & );

		Header &	operator=( Header const & );

		std::string	getResponse( void ) ;

		void		parseHeader( char buffer[] );
		void		createResponse( ConfigItem * item );

		struct Direc {
			std::string	root;
			std::string 	path;
			std::string	autoindex;
			std::vector<std::string>	indexes;
		};

private:
		void				_parseFirstLine( std::string, std::string );
		void				_parseFirstLine( std::string );
		std::string			_setParam( std::string ) ;
		std::string			_setContentType( std::string );
		std::string			_getDate( time_t timestamp );
		std::string			_getLastModified( std::string path );
		std::string			_replace( std::string in, std::string s1, std::string s2 );
		std::string			_getIndex( std::string path, std::vector<std::string> indexes );
		void        		_genErrorPage( std::string file, std::string code, std::string msg, std::string sentence );
		void			   	_noAutoIndexResponse( std::string path, std::stringstream & buf );
		void				_autoIndexResponse( std::string path, std::stringstream & buf );
		bool        		_isFolder( std::string path );
		bool        		_haveLocation( std::string requestPath, std::string location);
		Header::Direc	_getDirectives(ConfigItem * item, std::string suffix);

		std::map<std::string, std::string>	_headerParam;
		std::string	_response;
};
