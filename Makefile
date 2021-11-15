NAME		= webserv

SRCS		= $(addprefix src/webserv/, main.cpp Server.cpp Header.cpp config-parser/Lexer.cpp config-parser/ConfigParser.cpp config-parser/validator.cpp config-parser/ConfigItem.cpp config-parser/parser.cpp)

SRCS		+= $(addprefix src/, utils/Formatter.cpp utils/string.cpp)

SRCS		+= $(addprefix src/logger/, Logger.cpp)

SRCS		+= $(addprefix src/http/, status.cpp)

HEADER		= $(addprefix include/webserv/, Server.hpp Header.hpp Constants.hpp config-parser/Lexer.hpp config-parser/ConfigParser.hpp config-parser/validator.hpp config-parser/ConfigItem.hpp)

HEADER		+= $(addprefix include/, utils/Formatter.hpp utils/string.hpp)
HEADER		+=  $(addprefix include/logger/, Logger.hpp)

HEADER		+= $(addprefix include/http/, status.hpp)

OBJS		= $(SRCS:.cpp=.o)

FLAGS		= -Wall -Wextra -Werror -std=c++98 -Iinclude/webserv/ -Iinclude

CC			= @c++

RM			= @rm -f

OK			= "\r[ \033[0;32mok\033[0m ]"

FORMATTER   := clang-format

%.o:%.cpp	$(HEADER)
		$(CC) $(FLAGS) $< -c -o $@
		@printf "CC\t$@\n"
			
all:		$(NAME)

format:
	@$(FORMATTER) -i $(SRCS) $(HEADER) > /dev/null
	@printf "Codebase formatted using $(FORMATTER)\n"

$(NAME):	$(OBJS) $(HEADER)
		$(CC) $(FLAGS) $(OBJS) -o $@
		@printf "LD\t$@\n"

clean:
		$(RM) $(OBJS)

fclean:		clean
		$(RM) $(NAME)

re:			fclean all

run:		re
		@echo "$(NAME) running..."
		@./$(NAME) "./asset/config/simple.conf"

.PHONY:	all clean fclean re run
