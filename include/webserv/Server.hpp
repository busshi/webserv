/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aldubar <aldubar@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/10/05 11:04:48 by aldubar           #+#    #+#             */
/*   Updated: 2021/10/05 15:15:40 by aldubar          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

class	Server {

	public:
		Server( void );
		Server( int port );
		Server( Server const & src );
		~Server( void );

		Server &	operator=( Server const & rhs );

		void		start( void );
		void		parseHeader( void );
		void		sendResponse( void );
		void		stop( void );

	private:
		int			_port;
		int			_socketFd;
		int			_maxConnexion;
		int			_connexion;
};

#endif
