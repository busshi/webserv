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

SRCS		+= $(addprefix src/, main.cpp hosts.cpp lifecycle.cpp hooks.cpp create_response.cpp Directives.cpp HttpParser.cpp FileUploader.cpp Timer.cpp error_handling.cpp)

SRCS		+= $(addprefix src/config/, Lexer.cpp ConfigParser.cpp validator.cpp ConfigItem.cpp parser.cpp)

SRCS		+= $(addprefix src/utils/, Formatter.cpp Logger.cpp string.cpp os.cpp ErrorPageGenerator.cpp BinBuffer.cpp)

SRCS		+= $(addprefix src/http/, header.cpp status.cpp message.cpp request.cpp response.cpp FormDataParser.cpp Exception.cpp method.cpp)

SRCS		+= $(addprefix src/cgi/, cgi.cpp)

HEADER		= $(addprefix include/, core.hpp Directives.hpp Constants.hpp HttpParser.hpp FileUploader.hpp Buffer.hpp Timer.hpp)

HEADER		+= $(addprefix include/config/, validator.hpp ConfigParser.hpp ConfigItem.hpp Lexer.hpp)

HEADER		+= $(addprefix include/utils/, Formatter.hpp string.hpp os.hpp ErrorPageGenerator.hpp Logger.hpp BinBuffer.hpp)

HEADER		+= $(addprefix include/http/, header.hpp status.hpp message.hpp FormDataParser.hpp Exception.hpp method.hpp)

HEADER		+= $(addprefix include/cgi/, cgi.hpp)

OBJS		= $(SRCS:.cpp=.o)

CXX_FLAGS	= -g3 -Wall -Wextra -Werror -std=c++98 -Iinclude/webserv/ -Iinclude -fsanitize=address -Ihttp-parser $(if $(LOGGER), -DLOGGER, )

CC			= @c++

RM			= @rm -rf

OK			= "\r[ \033[0;32mok\033[0m ]"

FORMATTER   := clang-format

%.o:%.cpp $(HEADER)
		$(CC) $(CXX_FLAGS) $< -c -o $@
		@printf "\033[1;33mCC\033[0;m\t$@\n"

$(NAME): http-parser http-parser/libhttpparser.a $(OBJS) $(HEADER)
	$(CC)  $(OBJS) -fsanitize=address -o $@
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

redoc: cleandoc doc
	@printf "Doc REgenerated successfully.\n"

.PHONY: doc

http-parser:
	git clone https://github.com/aurelien-brabant/http-parser.git

http-parser/libhttpparser.a:
	make re -C http-parser
