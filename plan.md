# Webserv

TL;DR: Build a C++98 HTTP/1.1 compliant web server with configuration file parsing, non-blocking I/O using poll(), support for GET/POST/DELETE methods, file uploads, CGI execution, and custom error pages.

**Steps**
1. **Project Setup & Constraints**: Create project directory structure (src/, include/, config/), write Makefile with required rules and flags (-Wall -Wextra -Werror -std=c++98), ensure no external libraries are used.
2. **Configuration Parser**: Implement a parser for the configuration file to handle network limits (interface:port, max body size), error pages, routing rules (methods, redirections, root directories), directory listing, file uploads, and CGI configurations.
3. **Server Architecture (Non-Blocking I/O)**: Develop the core server loop using poll() for multiplexing all I/O operations, set sockets to non-blocking mode, handle multiple clients simultaneously without blocking.
4. **HTTP Handling & Routing**: Implement parsing of HTTP requests for GET, POST, DELETE; handle routing based on config, serve static files, manage file uploads, ensure accurate status codes and default error pages.
5. **CGI (Common Gateway Interface)**: Add support for CGI execution using fork(), set up environment variables, handle chunked requests by un-chunking for CGI, read CGI output until EOF if no content-length.
6. **Bonus Features (Optional)**: Implement cookies and session management, support multiple CGI types.

**Relevant files**
- `Makefile` — Build script with compilation rules.
- `src/main.cpp` — Entry point for the server.
- `include/Config.hpp`, `src/Config.cpp` — Configuration file parser.
- `include/Server.hpp`, `src/Server.cpp` — Server class handling networking and I/O.
- `include/HttpRequest.hpp`, `src/HttpRequest.cpp` — HTTP request parsing.
- `include/HttpResponse.hpp`, `src/HttpResponse.cpp` — HTTP response generation.
- `include/CGI.hpp`, `src/CGI.cpp` — CGI execution logic.
- `config/default.conf` — Example configuration file.

**Verification**
1. Compile the project using `make` and ensure no errors with -Wall -Wextra -Werror.
2. Test configuration parsing with a sample config file.
3. Use curl to test basic GET requests for static files.
4. Test POST for file uploads.
5. Test CGI execution with a simple script (e.g., PHP).
6. Run multiple concurrent requests to verify non-blocking behavior.
7. Check error handling with invalid requests.

**Potential Decisions**
- Use `poll()` as the I/O multiplexer for portability.
- Stick to C++98 standard and no external libraries.
- Handle CGI with `fork()` and pipes for input/output.
- Default configuration file path if none provided.

**Further Considerations**
1. Ensure cross-platform compatibility (Linux/MacOS) for poll() usage.
2. Implement robust error handling to prevent crashes.
3. Consider security aspects like path traversal prevention.

# Breakdown

## Phase 1: Project Setup & Constraints

* **Language & Standard:** The entire project must be written in C++ 98 and must compile with the `-std=c++98` flag.
* **Forbidden Tools:** You cannot use any external libraries or Boost libraries. You cannot `execve` another web server. 
* **Compilation:** Provide a `Makefile` with the rules `$(NAME)`, `all`, `clean`, `fclean`, and `re`. Use flags `-Wall -Wextra -Werror -std=c++98`.
* **Stability:** The program must not crash or terminate unexpectedly under any circumstances, including running out of memory.

## Phase 2: The Configuration Parser

Your server must be driven by a configuration file passed as an argument (or a default path). You must be able to configure:

* **Network Limits:** Define `interface:port` pairs for the server to listen on and set maximum allowed client request body sizes.
* **Error Handling:** Set up custom default error pages.
* **Routing Rules:** For specific URLs, define accepted HTTP methods, HTTP redirections, and the directory where files should be searched for.
* **Directory Behavior:** Toggle directory listing on or off, and specify a default file to serve if a directory is requested.
* **File Uploads:** Authorize client file uploads and define where they are stored.
* **CGI Execution:** Configure CGI execution based on file extensions (e.g., `.php`).

## Phase 3: Server Architecture (Non-Blocking I/O)

The core networking logic must handle multiple clients simultaneously without hanging.

* **Single Multiplexer:** You must use a single `poll()` (or an equivalent like `select`, `kqueue`, or `epoll`) for all I/O operations between clients and the server, including the listening socket.
* **Non-Blocking Operations:** All I/O that can wait for data (sockets, pipes) must be non-blocking. You must never use `read/recv` or `write/send` without first checking readiness via `poll()`. 
* **Exceptions:** Regular disk files do not require readiness notifications through `poll()`. Checking `errno` after a read/write to adjust server behavior is strictly forbidden.
* **MacOS Specifics:** If you are on MacOS, you can use `fcntl()` for non-blocking behavior, but strictly only with the `F_SETFL`, `O_NONBLOCK`, and `FD_CLOEXEC` flags.

## Phase 4: HTTP Handling & Routing

* **Methods:** Implement at least the `GET`, `POST`, and `DELETE` HTTP methods.
* **Content Delivery:** The server must be able to serve a fully static website and allow clients to upload files.
* **Accuracy:** Ensure your HTTP response status codes are accurate and default error pages are provided if missing.

## Phase 5: CGI (Common Gateway Interface)

* **Process Management:** You are allowed to use `fork` exclusively for executing CGI scripts.
* **Environment:** The full request and client arguments must be available to the CGI via environment variables.
* **Execution Context:** The CGI must be run in the correct directory to allow for relative path file access. You must support at least one CGI (like php-CGI or Python).
* **Data Handling:** For chunked requests, the server must un-chunk them because the CGI expects an EOF as the end of the body. The same applies to reading CGI output; if no `content_length` is returned, EOF marks the end of the data.

## Phase 6: Bonus Features

* **Cookies & Sessions:** Support cookies and session management, providing simple examples to demonstrate it.
* **Multiple CGIs:** Handle multiple different types of CGI scripts.