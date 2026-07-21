# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: ypua <ypua@student.42.fr>                  +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/07/11 20:58:40 by lyanga            #+#    #+#              #
#    Updated: 2026/07/21 20:11:42 by ypua             ###   ########.fr        #
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
			FileDescriptor.cpp \
			Socket.cpp 

# Map files to their respective folders
SRCS = $(addprefix $(SRC_DIR)/, $(SRC_FILES))
OBJS = $(addprefix $(OBJ_DIR)/, $(SRC_FILES:.cpp=.o))
DEPS = $(addprefix $(DEP_DIR)/, $(SRC_FILES:.cpp=.d))

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR) $(DEP_DIR)
	$(CXX) $(CXXFLAGS) -MF $(DEP_DIR)/$*.d -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

x: $(NAME) clean
	@echo =================================================
	./$(NAME) configs/commenthell.conf
	@echo =================================================
	./$(NAME) configs/commenthell1.conf

	@echo this returned $?
	@echo =================================================
	./$(NAME) configs/commenthell2.conf

-include $(DEPS)

.PHONY: all clean fclean re x