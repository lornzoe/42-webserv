NAME = webserv

SRC_DIR = src

MAIN_SRC_DIR = $(SRC_DIR)
MAIN_SRC_BASE = main WSApp			\
Server Listener Client Poller		\
w_utils

HEADER_BASE = WSApp					\
Server Listener Client Poller		\
w_utils								\
w_eventCtx w_logger

HEADERS = $(addprefix $(INCLD_DIR)/, $(addsuffix .hpp, $(HEADER_BASE)))

# OTHR_SRC_DIR = $(SRC_DIR)/others
# OTHR_SRC_BASE = Contact PhoneBook

SRCS = \
	$(addprefix $(MAIN_SRC_DIR)/, $(addsuffix .cpp, $(MAIN_SRC_BASE)))
# 	$(addprefix $(OTHR_SRC_DIR)/, $(addsuffix .cpp, $(OTHR_SRC_BASE)))

OBJ_DIR = objs
OBJS = $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SRCS))

COMPILE = c++
CFLAGS = -std=c++98 -Wall -Wextra -pedantic -Werror
INCLD_DIR = include
INCLUDES = -I$(INCLD_DIR)

ifdef DEBUG
	CFLAGS += -DDEBUG
endif

all: $(NAME)

$(NAME): $(OBJS)
	$(COMPILE) $(CFLAGS) $(OBJS) -o $@

$(OBJ_DIR)/%.o: %.cpp $(HEADERS)
	mkdir -p $(dir $@)
	$(COMPILE) -c $(CFLAGS) $(INCLUDES) $< -o $@

.PHONY:	all re fclean clean

#Cleanup
re: fclean all

fclean: clean
	rm -rf $(NAME)

clean:
	rm -rf $(OBJ_DIR)
