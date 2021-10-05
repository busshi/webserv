/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aldubar <aldubar@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/10/05 11:04:48 by aldubar           #+#    #+#             */
/*   Updated: 2021/10/05 12:38:49 by aldubar          ###   ########.fr       */
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

		int			start( void);

	private:
		int		_port;
};

#endif
