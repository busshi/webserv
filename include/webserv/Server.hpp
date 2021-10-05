/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aldubar <aldubar@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/10/05 11:04:48 by aldubar           #+#    #+#             */
/*   Updated: 2021/10/05 14:44:55 by aldubar          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include <netinet/in.h>

class	Server {

	public:
		Server( void );
		Server( int port );
		Server( Server const & src );
		~Server( void );

		Server &	operator=( Server const & rhs );

		void		init( void );
		void		start( void);
		void		stop( void);

	private:
		int			_port;
		int			_socketFd;
		int			_maxConnexion;
		sockaddr_in	_sockaddr;
};

#endif
