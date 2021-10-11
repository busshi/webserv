/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aldubar <aldubar@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/10/05 11:04:48 by aldubar           #+#    #+#             */
/*   Updated: 2021/10/05 18:10:18 by aldubar          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <vector>

class Server
{
  public:
    Server(void);
    Server(Server const& src);
    ~Server(void);

    Server& operator=(Server const& rhs);

    void init(char const* argv);
    void start(void);
    void parseHeader(char buffer[]);
    void sendResponse(void);
    void stop(void);

    void display(void);

  private:
    int _port;
    int _socketFd;
    int _maxConnexion;
    int _connexion;

    std::string _method;
    std::string _path;
};

#endif
