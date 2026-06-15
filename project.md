*This project has been created as part of the 42 curriculum by \<login1\>[, \<login2\>].*

---

# webserv

An HTTP/1.1 server written in C++98 from scratch. No external libraries. No Boost.
The server uses a single non-blocking `poll()` loop to handle multiple simultaneous
client connections without threads.

---

## Description

`webserv` implements a subset of the HTTP/1.1 protocol sufficient to serve a fully
static website, handle file uploads and deletions, execute CGI scripts (Python, PHP,
etc.), and redirect requests. It can listen on multiple ports simultaneously, each
with its own root directory, error pages, and location rules.

Key features:
- Single-threaded, non-blocking I/O via `poll()`
- Configurable via an NGINX-inspired config file
- Static file serving with correct MIME types
- Directory listing (`autoindex`)
- File upload (POST) and deletion (DELETE)
- HTTP redirects (301/302)
- CGI execution via `fork`/`execve` (Python, php-cgi, …)
- Chunked transfer encoding (de-chunking on receive)
- Default and custom error pages
- Multiple server blocks / ports in one process
- Client timeout detection

---

## Instructions

### Requirements

- A C++98-compatible compiler (`c++` / `g++` / `clang++`)
- GNU Make
- Python 3 (only needed to run CGI scripts and the test suite)

### Build

```bash
make
```

This produces the `./webserv` binary. To rebuild from scratch:

```bash
make re
```

To remove objects without removing the binary:

```bash
make clean
```

To remove everything:

```bash
make fclean
```

### Run

```bash
./webserv [config_file]
```

If no config file is given, `config/default.conf` is used.

**Examples:**

```bash
# Default config — server on :8080 and :8081
./webserv

# Explicit path
./webserv config/default.conf

# Minimal single-port config
./webserv config/minimal.conf

# CGI-focused config on port 9090
./webserv config/cgi_test.conf
```

Once running, open your browser at `http://localhost:8080/`.

### Run the test suite

Start the server first, then in another terminal:

```bash
python3 test_webserv.py
```

The test script covers: static serving, 404/405 errors, directory listing, file
upload/download/delete, HTTP redirect, CGI (GET + POST), oversized body rejection,
50 sequential requests, 20 concurrent connections, and the second server on port 8081.

### Quick manual tests with curl

```bash
# GET static file
curl -v http://localhost:8080/index.html

# Upload a file
curl -v -X POST http://localhost:8080/upload \
     -H 'Content-Disposition: attachment; filename="hello.txt"' \
     --data-binary 'Hello, webserv!'

# Browse uploaded files
curl http://localhost:8080/files

# Delete a file
curl -v -X DELETE http://localhost:8080/upload/hello.txt

# Follow a redirect
curl -L http://localhost:8080/old-page

# Run CGI script
curl "http://localhost:8080/cgi-bin/info.py?name=curl&year=2024"

# POST to CGI echo script
curl -X POST http://localhost:8080/cgi-bin/echo.py \
     -H 'Content-Type: application/json' \
     -d '{"test": true}'
```

---

## Project Structure

```
webserv/
├── Makefile
├── README.md
├── test_webserv.py        # Automated test suite
│
├── include/               # Header files
│   ├── Config.hpp         # Configuration structures and parser
│   ├── HttpRequest.hpp    # HTTP request parser
│   ├── HttpResponse.hpp   # HTTP response builder
│   ├── Client.hpp         # Per-connection state and request handlers
│   ├── Server.hpp         # poll() event loop
│   └── Utils.hpp          # Small helper functions
│
├── src/                   # Source files
│   ├── main.cpp           # Entry point
│   ├── Config.cpp         # Config file parser
│   ├── HttpRequest.cpp    # Incremental HTTP request parser
│   ├── HttpResponse.cpp   # Response serialisation and MIME types
│   ├── Client.cpp         # GET / POST / DELETE / CGI / directory listing
│   ├── Server.cpp         # Non-blocking poll() loop
│   └── Utils.cpp          # trim, split, readFile, isFile, isDirectory …
│
├── config/
│   ├── default.conf       # Main config (ports 8080 + 8081, CGI, upload)
│   ├── minimal.conf       # Minimal single-port static-only config
│   └── cgi_test.conf      # CGI-focused config on port 9090
│
└── www/
    ├── html/              # Document root
    │   ├── index.html     # Test page with upload/delete/CGI demo UI
    │   └── errors/        # Custom error pages
    │       ├── 404.html
    │       └── 500.html
    ├── cgi-bin/           # CGI scripts
    │   ├── info.py        # Shows environment variables and query params
    │   └── echo.py        # Echoes POST body as JSON
    └── uploads/           # Files uploaded by clients land here
```

---

## Configuration File

See [Configuration File Reference](#configuration-file-reference) below, or read the
heavily-commented `config/default.conf` directly.

### Configuration File Reference

The config file uses an NGINX-inspired block syntax. Lines starting with `#` are
comments. Each `server { }` block creates one virtual server bound to one port.

#### Server-level directives

| Directive | Example | Description |
|---|---|---|
| `listen` | `listen 0.0.0.0:8080;` | Interface and port to bind. Use `0.0.0.0` for all interfaces. |
| `server_name` | `server_name mysite.com;` | Optional hostname (for future virtual hosting). |
| `root` | `root /var/www/html;` | Default filesystem root for serving files. |
| `index` | `index index.html;` | Default file when URI points to a directory. |
| `client_max_body_size` | `client_max_body_size 10M;` | Maximum allowed request body size. Suffix `K`=kilobytes, `M`=megabytes. Requests exceeding this get a 413 response. |
| `error_page` | `error_page 404 /errors/404.html;` | Custom error page path (relative to root). Can repeat for different codes. |

#### Location blocks

A `location /prefix { }` block applies rules to all URIs that start with `/prefix`.
The longest matching prefix wins.

| Directive | Example | Description |
|---|---|---|
| `methods` | `methods GET POST;` | Whitelist of allowed HTTP methods. Omitting this allows all methods. |
| `root` | `root /srv/static;` | Override the filesystem root for this location. |
| `index` | `index index.html;` | Override the default index file. |
| `autoindex` | `autoindex on;` | Generate a directory listing when no index file is found. Default: `off`. |
| `upload_dir` | `upload_dir /var/uploads;` | Directory where POSTed files are saved. Required for file upload to work. |
| `return` | `return 301 /new/path;` | Issue an HTTP redirect. First arg is the status code (optional), second is the target URL. |
| `cgi_extension` | `cgi_extension .py;` | File extension that triggers CGI execution. |
| `cgi_path` | `cgi_path /usr/bin/python3;` | Path to the CGI interpreter binary. |

#### Complete example

```nginx
server {
    listen      0.0.0.0:8080;
    server_name example.com;
    root        /var/www/html;
    index       index.html;
    client_max_body_size 20M;

    error_page  404 /errors/404.html;
    error_page  500 /errors/500.html;

    # Serve static files at root
    location / {
        methods     GET;
        autoindex   off;
    }

    # Upload endpoint — POST saves, DELETE removes
    location /upload {
        methods     POST DELETE;
        upload_dir  /var/www/uploads;
    }

    # Browse uploads
    location /files {
        methods     GET;
        root        /var/www/uploads;
        autoindex   on;
    }

    # Python CGI scripts
    location /cgi-bin {
        methods         GET POST;
        root            /var/www/cgi-bin;
        cgi_extension   .py;
        cgi_path        /usr/bin/python3;
    }

    # Legacy URL redirect
    location /old {
        return  301 /;
    }
}

# Second server on different port
server {
    listen  0.0.0.0:9000;
    root    /var/www/api;
    index   index.html;

    location / {
        methods     GET POST;
        autoindex   on;
    }
}
```

---

## How Non-Blocking I/O Works

The subject requires that **all I/O goes through a single `poll()` call** and that
**sockets are never read or written without `poll()` indicating readiness**.

Here is the flow in `Server::run()`:

```
┌─────────────────────────────────────────────────┐
│ Build pollfd array:                             │
│   [listen_fd_1, POLLIN]                         │
│   [listen_fd_2, POLLIN]                         │
│   [client_fd_1, POLLIN or POLLOUT]              │
│   [client_fd_2, POLLIN or POLLOUT]  ...         │
└───────────────────┬─────────────────────────────┘
                    │
               poll(fds, n, 1000ms)
                    │
         ┌──────────┴──────────┐
         │  For each ready fd  │
         └──────────┬──────────┘
                    │
         ┌──────────┴───────────────────────┐
         │ Listening fd + POLLIN?           │
         │  -> accept() new client          │
         │  -> set client fd O_NONBLOCK     │
         │  -> add Client to map            │
         ├──────────────────────────────────┤
         │ Client fd + POLLIN?              │
         │  -> Client::doRead()             │
         │     recv() bytes into buffer     │
         │     feed to HttpRequest parser   │
         │     when complete: buildResponse │
         │     -> switch state to WRITING   │
         ├──────────────────────────────────┤
         │ Client fd + POLLOUT?             │
         │  -> Client::doWrite()            │
         │     send() bytes from writeBuffer│
         │     track offset for partial send│
         │     when done: CLIENT_DONE       │
         └──────────────────────────────────┘
```

`errno` is never inspected to change server behaviour after a read or write
(the subject forbids this). `EAGAIN`/`EWOULDBLOCK` from `recv`/`send` are
simply handled by returning without error and waiting for the next `poll()`.

---

## CGI Notes

CGI scripts are executed via `fork()` + `execve()`. The parent server:

1. Creates two pipes: one for stdin (request body → CGI), one for stdout (CGI output → server).
2. Forks. Child: `dup2` the pipes onto `STDIN_FILENO`/`STDOUT_FILENO`, `chdir` to the script directory, `execve` the interpreter.
3. Parent: writes the request body to the CGI's stdin pipe, reads all output from the stdout pipe, `waitpid` to reap the child.
4. Parses the CGI output: headers up to the blank line, then body. `Status:` header sets the response code.

CGI scripts must write their own headers. At minimum:

```
Content-Type: text/html\r\n
\r\n
<body>
```

For chunked POST bodies the server de-chunks before passing to the CGI, so the CGI
always receives a flat body with a correct `CONTENT_LENGTH`.

---

## Resources

- [RFC 7230 — HTTP/1.1: Message Syntax and Routing](https://datatracker.ietf.org/doc/html/rfc7230)
- [RFC 7231 — HTTP/1.1: Semantics and Content](https://datatracker.ietf.org/doc/html/rfc7231)
- [RFC 3875 — The Common Gateway Interface (CGI)](https://datatracker.ietf.org/doc/html/rfc3875)
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)
- [NGINX Configuration Guide](https://nginx.org/en/docs/beginners_guide.html)
- *Computer Networking: A Top-Down Approach* — Kurose & Ross (Chapter 2, Application Layer)

**How AI was used in this project:**

AI (Claude) was used to assist with:
- Explaining the chunked transfer encoding algorithm and edge cases in the de-chunking logic.
- Reviewing the poll() loop structure for correctness against the subject requirements.
- Suggesting the incremental parser state machine design for HttpRequest.
- Generating boilerplate (CGI environment variable list, MIME type table).

All generated content was reviewed, understood, and adapted before use. The final
implementation was written and is fully understood by the team.
