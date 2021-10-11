#include "Server.hpp"
#include <iostream>
#include "webserv/config-parser/Lexer.hpp"

int		main( int ac, char **av ) {

	if (ac != 2) {

		//std::cout << "Usage: ./webserv [path to config file]" << std::endl;
		std::cout << "Usage: ./webserv [port to bind]" << std::endl;
		return 0;
	}

	Server	server;
	Lexer lexer(
			"{               "
			" { }                     {}{}{}{}}"
			"}                    "
		);

	try {
		(void)av;
		(void)ac;

		while (lexer.next().getType() != Lexer::END_OF_FILE);

		/*
		server.init(av[1]);
		server.start();
		server.stop();
		*/
	}
	catch (Lexer::LexerException& e) {
		e.printFormatted(std::cerr) << "\n";
	}
	catch (std::exception & e) {
		std::cout << e.what() << std::endl;
	}

	return 0;
}
