#pragma once

#include <string>

class	Header {

	public:
		Header( void );
		~Header( void );

		Header( Header const & );

		Header &	operator=( Header const & );

		std::string	getResponse( void ) ;

		void		parseHeader( char buffer[], std::string rootPath );
		void		setContentType( std::string );
		void		createResponse( void );

	private:
		std::string	_rootPath;
		std::string	_method;
		std::string	_path;
		std::string	_response;
		std::string	_statusCode;
		std::string	_contentType;
		std::string	_contentLen;
		std::string	_http;
		std::string	_host;
		std::string	_userAgent;
		std::string	_accept;
		std::string	_acceptLanguage;
		std::string	_acceptEncoding;
		std::string	_connection;
		std::string	_referer;
};
