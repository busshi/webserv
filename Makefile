NAME		= webserv

DOXYGEN_OUT			:= ./doxygen
DOXYGEN_CONF_DIR	:= ./doxygen-conf
DOXYGEN_CONF_FILE	:= doxygen.conf

UNAME				:= $(shell uname)

ifeq ($(UNAME), Linux)
	OPEN := xdg-open
else
	OPEN := open
endif

SRCS		= $(addprefix src/webserv/, main.cpp Server.cpp Header.cpp Directives.cpp config-parser/Lexer.cpp config-parser/ConfigParser.cpp config-parser/validator.cpp config-parser/ConfigItem.cpp config-parser/parser.cpp)
SRCS		+= $(addprefix src/, utils/Formatter.cpp utils/string.cpp utils/os.cpp)
SRCS		+= $(addprefix src/logger/, Logger.cpp)
SRCS		+= $(addprefix src/http/, status.cpp message.cpp)

HEADER		= $(addprefix include/webserv/, Server.hpp Header.hpp Directives.hpp Constants.hpp config-parser/Lexer.hpp config-parser/ConfigParser.hpp config-parser/validator.hpp config-parser/ConfigItem.hpp)
HEADER		+= $(addprefix include/, utils/Formatter.hpp utils/string.hpp utils/os.hpp)
HEADER		+=  $(addprefix include/logger/, Logger.hpp)
HEADER		+= $(addprefix include/http/, status.hpp message.hpp)

OBJS		= $(SRCS:.cpp=.o)

CXX_FLAGS	= -Wall -Wextra -Werror -std=c++98 -Iinclude/webserv/ -Iinclude -fsanitize=address

CC			= @c++

RM			= @rm -rf

OK			= "\r[ \033[0;32mok\033[0m ]"

FORMATTER   := clang-format

%.o:%.cpp $(HEADER)
		$(CC) $(CXX_FLAGS) $< -c -o $@
		@printf "\033[1;33mCC\033[0;m\t$@\n"

$(NAME): $(OBJS) $(HEADER)
	$(CC) $(CXX_FLAGS) $(OBJS) -o $@
	@printf "BIN \033[1;32m=>\033[0m \t$@\n"
			
all: $(NAME)
.PHONY: all

format:
	@$(FORMATTER) -i $(SRCS) $(HEADER) > /dev/null
	@printf "Codebase formatted using $(FORMATTER)\n"
.PHONY: format

clean:
		$(RM) $(OBJS)
.PHONY: clean

fclean: clean
	$(RM) $(NAME)
.PHONY: fclean

re:	fclean all
.PHONY: re

run: re
	@./$(NAME) "./asset/config/simple.conf"
.PHONY: run

$(DOXYGEN_OUT)/html/index.html:
	@printf "Will generate documentation using doxygen...\n"
	@doxygen $(DOXYGEN_CONF_DIR)/$(DOXYGEN_CONF_FILE)
	@printf "Documentation generated!\n"

doc: $(DOXYGEN_OUT)/html/index.html
	@($(OPEN) $(DOXYGEN_OUT)/html/index.html 2> /dev/null &)
	@printf "Now displaying documentation in your browser's tab\nProcess has been sent to the background.\n";

cleandoc:
	$(RM) $(DOXYGEN_OUT)

redoc: docclean doc
	@printf "Doc REgenerated successfully.\n"

.PHONY: doc