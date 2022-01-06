NAME		= webserv

SRCS		+= $(addprefix src/, main.cpp hosts.cpp lifecycle.cpp hooks.cpp Directives.cpp FileUploader.cpp Timer.cpp error_handling.cpp process_request.cpp)

SRCS		+= $(addprefix src/config/, Lexer.cpp ConfigParser.cpp validator.cpp ConfigItem.cpp parser.cpp)

SRCS		+= $(addprefix src/utils/, Formatter.cpp Logger.cpp string.cpp os.cpp ErrorPageGenerator.cpp BinBuffer.cpp)

SRCS		+= $(addprefix src/http/, header.cpp status.cpp message.cpp request.cpp response.cpp FormDataParser.cpp Exception.cpp method.cpp MessageParser.cpp uri.cpp)

SRCS		+= $(addprefix src/event/, cgi.cpp upload.cpp client.cpp server.cpp utils.cpp)

SRCS		+= $(addprefix src/cgi/, cgi.cpp)

HEADER		= $(addprefix include/, core.hpp Directives.hpp Constants.hpp FileUploader.hpp Buffer.hpp Timer.hpp)

HEADER		+= $(addprefix include/config/, validator.hpp ConfigParser.hpp ConfigItem.hpp Lexer.hpp)

HEADER		+= $(addprefix include/utils/, Formatter.hpp string.hpp os.hpp ErrorPageGenerator.hpp Logger.hpp BinBuffer.hpp)

HEADER		+= $(addprefix include/http/, header.hpp status.hpp message.hpp FormDataParser.hpp Exception.hpp method.hpp MessageParser.hpp uri.hpp)

HEADER		+= $(addprefix include/cgi/, cgi.hpp)

OBJS		= $(SRCS:.cpp=.o)

CXX_FLAGS	= -g3 -Wall -Wextra -Werror -std=c++98 -Iinclude/webserv/ -Iinclude -fsanitize=address $(if $(LOGGER), -DLOGGER, )

CC			= @c++

RM			= @rm -rf

FORMATTER   := clang-format

%.o:%.cpp $(HEADER)
		$(CC) $(CXX_FLAGS) $< -c -o $@
		@printf "\033[1;33mCC\033[0;m\t$@\n"

$(NAME): $(HEADER) $(OBJS)
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
