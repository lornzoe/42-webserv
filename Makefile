# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: ypua <ypua@student.42.singapore.sg>        +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/06/30 20:58:00 by ypua              #+#    #+#              #
#    Updated: 2026/07/19 17:53:29 by ypua             ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = webserv

# CPP Files
CPP_FILES = main.cpp
O_FILES = $(CPP_FILES:.cpp=.o)

# Add dependency files
D_FILES = $(O_FILES:.o=.d)

# Commands
CXX = c++
# -MMD generates .d files, -MP prevents errors if a header gets deleted
CFLAGS = -Wall -Werror -Wextra -std=c++98 -MMD -MP
RM = rm -f

# Rules
all: $(NAME)

$(NAME): $(O_FILES)
	$(CXX) $(CFLAGS) $(O_FILES) -o $@

$(O_FILES): %.o: %.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(O_FILES) $(D_FILES)

fclean: clean
	$(RM) $(NAME)

re: fclean all

# Include dependency files if they exist
-include $(D_FILES)

.PHONY: all clean fclean re
