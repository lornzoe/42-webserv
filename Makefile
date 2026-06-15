# ============================================================================
# Makefile for webserv — HTTP server in C++98
# ============================================================================
# Rules required by the subject: $(NAME), all, clean, fclean, re
# Flags required: -Wall -Wextra -Werror -std=c++98
#
# How it works:
#   1. CXX / CXXFLAGS: compiler + flags
#   2. SRCS: all .cpp source files
#   3. OBJS: each .cpp becomes a .o in the same directory
#   4. $(NAME): link all .o files into the final binary
#   5. The dependency on $(OBJS) ensures only changed files are recompiled
#      (avoiding unnecessary relinking as required by the subject)
# ============================================================================

NAME    := webserv

CXX     := c++
CXXFLAGS := -Wall -Wextra -Werror -std=c++98

# Include path so headers in include/ are found
INCLUDES := -Iinclude

# All source files — add new .cpp files here
SRCS := src/main.cpp      \
        src/Config.cpp    \
        src/Utils.cpp     \
        src/HttpRequest.cpp \
        src/HttpResponse.cpp \
        src/Client.cpp    \
        src/Server.cpp

# Object files derived from source files (same path, .cpp -> .o)
OBJS := $(SRCS:.cpp=.o)

# ============================================================================
# Rules
# ============================================================================

# Default target: build the server binary
all: $(NAME)

# Link step: combine all object files into the executable
$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(NAME) $(OBJS)
	@echo "Built: $(NAME)"

# Compile step: each .cpp -> .o
# $< is the .cpp source, $@ is the .o target
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Remove compiled object files (source still there, binary still there)
clean:
	rm -f $(OBJS)

# Remove everything including the binary
fclean: clean
	rm -f $(NAME)

# Full rebuild: fclean then all
re: fclean all

# Declare non-file targets so make doesn't look for files named "all", etc.
.PHONY: all clean fclean re
