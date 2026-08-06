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
			DirectiveFactory.cpp \
			DirectiveRules.cpp \
			Directive.cpp \
			BlockDirective.cpp \
			SimpleDirective.cpp \
			ServerDirective.cpp \
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
			HttpStatus.cpp \
			\
			FileDescriptor.cpp \
			Socket.cpp

WSAPP_SRC_DIR = $(SRC_DIR)/wsapp
WSAPP_SRC_BASE = 	WSApp Poller Server Listener Client \
					w_utils

# Map files to their respective folders
SRCS =	$(addprefix $(SRC_DIR)/, $(SRC_FILES)) \
		$(addprefix $(WSAPP_SRC_DIR)/, $(addsuffix .cpp, $(WSAPP_SRC_BASE)))
OBJS =	$(addprefix $(OBJ_DIR)/, $(SRCS:.cpp=.o))
DEPS =	$(addprefix $(DEP_DIR)/, $(SRCS:.cpp=.d))

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@) $(dir $(DEP_DIR)/$*.d)
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
