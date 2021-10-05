NAME		= webserv

SRCS		= $(addprefix src/webserv/, main.cpp Server.cpp)

HEADER		= $(addprefix include/webserv/, Server.hpp)

OBJS		= $(SRCS:.cpp=.o)

FLAGS		= -Wall -Wextra -Werror -std=c++98 -Iinclude/webserv/

CC			= @clang++

RM			= @rm -f

OK			= "\r[ \033[0;32mok\033[0m ]"

%.o:%.cpp	$(HEADER)
		$(CC) $(FLAGS) $< -c -o $@
			
all:		$(NAME)

$(NAME):	$(OBJS) $(HEADER)
		@echo "[....] Compiling $(NAME)\c"
		$(CC) $(FLAGS) $(OBJS) -o $@
		@echo $(OK)

clean:
		@echo "[....] Removing $(NAME) objects\c"
		$(RM) $(OBJS)
		@echo $(OK)

fclean:		clean
		@echo "[....] Removing $(NAME)\c"
		$(RM) $(NAME)
		@echo $(OK)

re:			fclean all

.PHONY:	all clean fclean re
