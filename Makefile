# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: ypua <ypua@student.42.singapore.sg>        +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/07/11 20:58:40 by lyanga            #+#    #+#              #
#    Updated: 2026/08/09 16:05:28 by ypua             ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME     = webserv
CXX      = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -MMD -MP -Iinclude

# Folder structure definitions
SRC_DIR   = src
BUILD_DIR = build
OBJ_DIR   = $(BUILD_DIR)/obj
DEP_DIR   = $(BUILD_DIR)/deps

# Explicitly list your source files here (just the file names, no paths)
SRC_FILES = main.cpp \
			Config.cpp \
			QuoteTracker.cpp \
			DirectiveFactory.cpp \
			DirectiveRules.cpp \
			\
			Directive.cpp \
			BlockDirective.cpp \
			SimpleDirective.cpp \
			ServerDirective.cpp \
			LocationDirective.cpp \
			ListenDirective.cpp \
			ServerNameDirective.cpp \
			AliasDirective.cpp \
			RootDirective.cpp \
			IndexDirective.cpp \
			ClientMaxBodySizeDirective.cpp \
			ErrorPageDirective.cpp \
			ReturnDirective.cpp \
			LimitExceptDirective.cpp \
			\
			HttpRequest.cpp \
			HttpStatus.cpp \
			MimeTypes.cpp \
			\
			FileDescriptor.cpp \
			Socket.cpp

WSAPP_SRC_DIR = $(SRC_DIR)/wsapp
WSAPP_SRC_BASE = 	WSApp Poller Server Listener Client \
					w_utils
WSAPP_SRC_FILES =  $(addsuffix .cpp, $(WSAPP_SRC_BASE))

# Map files to their respective folders
SRCS =	$(addprefix $(SRC_DIR)/, $(SRC_FILES)) \
		$(addprefix $(WSAPP_SRC_DIR)/, $(WSAPP_SRC_FILES))
OBJS =	$(addprefix $(OBJ_DIR)/, $(SRCS:.cpp=.o))
DEPS =	$(addprefix $(DEP_DIR)/, $(SRCS:.cpp=.d))

# Config checker: only for seeing tokenisation and print info
CHECKER_NAME     = checker
CHECKER_SRC_FILES = $(filter-out main.cpp, $(SRC_FILES))
CHECKER_SRCS = $(addprefix $(SRC_DIR)/, $(CHECKER_SRC_FILES)) $(addprefix $(SRC_DIR)/, ConfigChecker.cpp)
CHECKER_OBJS = $(addprefix $(OBJ_DIR)/, $(CHECKER_SRCS:.cpp=.o))
CHECKER_DEPS = $(addprefix $(DEP_DIR)/, $(CHECKER_SRCS:.cpp=.d))

all: $(NAME) $(CHECKER_NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

$(CHECKER_NAME): $(CHECKER_OBJS)
	$(CXX) $(CXXFLAGS) $(CHECKER_OBJS) -o $(CHECKER_NAME)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@) $(dir $(DEP_DIR)/$*.d)
	$(CXX) $(CXXFLAGS) -MF $(DEP_DIR)/$*.d -c $< -o $@

x: $(NAME)
	./$(CHECKER_NAME) configs/basic.conf
	./$(NAME) configs/basic.conf

clean:
	rm -rf $(BUILD_DIR)

fclean: clean
	rm -f $(NAME) $(CHECKER_NAME)

re: fclean all

-include $(DEPS) $(CHECKER_DEPS)

.PHONY: all clean fclean re x checker
