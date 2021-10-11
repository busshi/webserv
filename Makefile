NAME		= webserv

SRCS		= $(addprefix src/webserv/, main.cpp Server.cpp config-parser/Lexer.cpp config-parser/ConfigParser.cpp)

HEADER		= $(addprefix include/webserv/, Server.hpp config-parser/Lexer.hpp config-parser/ConfigParser.hpp)

OBJS		= $(SRCS:.cpp=.o)

FLAGS		= -Wall -Wextra -Werror -std=c++98 -Iinclude/webserv/ -Iinclude

CC			= @clang++

RM			= @rm -f

OK			= "\r[ \033[0;32mok\033[0m ]"

%.o:%.cpp	$(HEADER)
		$(CC) $(FLAGS) $< -c -o $@
		@printf "CC\t$@\n"
			
all:		$(NAME)

$(NAME):	$(OBJS) $(HEADER)
		$(CC) $(FLAGS) $(OBJS) -o $@
		@printf "LD\t$@\n"

clean:
		$(RM) $(OBJS)

fclean:		clean
		$(RM) $(NAME)

re:			fclean all

run:		re
		@echo "Running $(NAME) on port 80..."
		@./$(NAME) 80

.PHONY:	all clean fclean re run
