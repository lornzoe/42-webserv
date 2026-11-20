_This project has been created as part of the 42 curriculum by lyanga, ypua, julhong_

# Webserv

## Description

Webserv is an HTTP/1.1 server we built from scratch in C++98 to understand how a browser request becomes a response.

**HTTP** (OSI application layer, layer 7) gives the browser and server a shared language. `GET` asks for a file, `200 OK` signals success, and `Content-Type` tells the browser how to interpret the body. The browser writes its request to a **socket**, its local endpoint for the connection. **TCP** (transport layer, layer 4) carries the bytes in order and retransmits lost data. At the other end, Webserv reads the request and writes the response through its own connected socket.

### What does our webserv do when you go to a URL (e.g.`/docs/guide.txt`)?

1. **Connect.** When the browser connects to port 8080, `accept()` gives the `Listener` a new socket for that connection. The listening socket stays available for the next browser. The connected socket is handed to a `Client` object, which tracks reads and writes for that connection across successive HTTP requests.
2. **Read.** A single HTTP request can arrive across several reads. `Client` buffers the bytes until the parser can read the request line, headers, and any body. For uploads, it uses `Content-Length` or chunked encoding to find the end of the body.
3. **Route.** The request line `GET /docs/guide.txt HTTP/1.1` asks for a file. The longest matching `location` is `/docs/`, whose configured root maps the URL to `showcase/www/files/docs/guide.txt`.
4. **Reply.** Webserv sends `200 OK`, a `Content-Type: text/plain` header, and the file contents. Ask for a missing file and it sends `404 Not Found` with the site's error page. Both responses follow the same write path, and the server keeps serving other clients.

One server process handles these connections with non-blocking sockets and a single Linux `epoll` loop. `epoll` reports which listeners have connections to accept and which client sockets or CGI pipes are ready to read or write. While one browser pauses halfway through an upload, the loop can send the guide to another. If a write sends only part of a response, the `Client` remembers where to resume.

### Features

- **Configuration.** NGINX-style `server` and `location` blocks define each site's `interface:port` pairs and directories, allowed methods, body size limits, redirects, and custom error pages.
- **Multiple ports.** One process can listen on several ports and serve different content on each.
- **Files.** `GET` serves static files. For a directory, the server looks for a configured index file and can list the contents if it finds none. On upload routes, `POST` accepts multipart file uploads and `DELETE` removes uploaded files.
- **Error handling.** Every error status has a built-in default page, so a site works even without `error_page` directives. Bodies over `client_max_body_size` get `413 Payload Too Large`.
- **No hanging requests.** Idle or stalled clients are timed out after 5 seconds (`408 Request Timeout`), and CGI scripts after 10 seconds (`504 Gateway Timeout`).
- **CGI.** The Common Gateway Interface lets a script generate the response. Chunked request bodies are unchunked before being passed to the script, which runs in its own directory so relative paths work.

#### Bonus

- **Sessions and cookies.** A `sid` cookie identifies a profile stored in server memory. Save a name on the profile page, then reload to see the name persist and the visit count increase.
- **Multiple CGI types.** The showcase maps `.py` to Python and `.pl` to Perl.

### Technical choices

- **`epoll` instead of `poll`.** As the subject allows any equivalent; `epoll` scales better with many connections.
- **One loop for all I/O.** Listening sockets, client sockets, CGI pipes, and the server's own stdin are all registered in the same `epoll` instance. Nothing is read from or written to a socket or pipe without `epoll` reporting it ready first.
- **No `errno` after I/O.** The return value of `read`/`recv`/`write`/`send` alone is used to determine the next action.
- **Main classes.**
  - `WSApp` owns the event loop and timeouts.
  - `Poller` wraps `epoll`.
  - `Server` and `Listener` hold each configured site and its listening sockets.
  - `Client` handles one connection.
  - `HttpRequest` / `HttpResponse` parse and build messages.
  - `CgiProcess` forks and talks to CGI scripts through pipes.
  - `SeshCtl` stores sessions.

## Instructions

### Quick start

```sh
make
./webserv showcase/full.conf
```

Open [the showcase](http://localhost:8080/) to try the upload form, CGI demos, and session profile. The same site also listens on port 8081, and a [second site](http://localhost:8082/) shows one process serving different content on another port. Routes and example requests are listed in the [showcase guide](showcase/README.md).

The showcase needs executable interpreters at `/usr/bin/python3` and `/usr/bin/perl`. If yours are elsewhere, update the two `cgi` paths in [the showcase configuration](showcase/full.conf).

### Requirements

Since webserv uses `epoll`, it builds and runs on Linux only, with `make` and a C++98 compiler.

### Compiling

```sh
make
```

Additional Makefile rules:

- `make clean` removes the `build/` directory, `make fclean` also removes the `webserv` and `checker` binaries, and `make re` rebuilds from scratch.
- `make checker` builds `checker`, which only parses a config file and reports whether it is valid: `./checker <config file>`.
- `make debug` rebuilds with debug logging and debug flags (`-DDEBUG -g -O0`).

### Usage

```sh
./webserv [config file]
```

Without an argument, Webserv loads [the basic configuration](dev/configs/basic.conf).

To stop the server, press `Ctrl+C` or `Ctrl+D`, or type `exit` in its terminal.

## Testing

### Configuration files

Each file in [`dev/configs/`](dev/configs/) exercises a specific feature:

| Feature | Config |
| --- | --- |
| Everything together | [`showcase/full.conf`](showcase/full.conf) |
| Static site, directory listing | `basic.conf`, `autoindex.conf` |
| Custom index file | `badindex.conf` |
| Uploads and `DELETE` | `upload.conf`, `upload_del.conf` |
| CGI and its working directory | `cgi.conf`, `cwd_cgi.conf` |
| Several ports / servers | `multi_listen.conf`, `multi_server.conf`, `multi_bind.conf` |
| Redirections | `redirections.conf`, `301throw.conf`, `301catch.conf` |
| `root` vs `alias` | `aliastest.conf` |
| Custom and default error pages | `error_page_test.conf` |
| Sessions and cookies (bonus) | `sessionDemo.conf` |
| Parser edge cases (comments, quotes, whitespace, one-liners) | `commenthell*.conf`, `quotations.conf`, `whitespacehell.conf`, `*oneliner.conf` |
| Config that must be rejected | `invalid.conf` |

To parse every config in `dev/configs/` at once:

```sh
make checker
```

```sh
scripts/check_configs.sh
```

### Scripts

The scripts in [`scripts/`](scripts/) target a server running on `localhost:8080`:

- `chunked_request_test.py` sends a chunked `POST` body.
- `incomplete_request_test.py` sends headers that never finish and waits for the server's reply.
- `delay_request_test.py` sends unfinished headers and then stays silent for 10 seconds. The server should answer `408 Request Timeout` after its 5-second limit instead of hanging.
- `test_error_page_directives.sh` starts Webserv with `error_page_test.conf` and checks custom and default error pages, including a malformed request.
- `test_cwd.py` is a CGI script that prints its working directory.
- `valgrind_loop.sh [binary]` runs `webserv` (or `checker`) under Valgrind to check for leaks.

### Stress test

With the showcase running, [Siege](https://github.com/JoeDog/siege) checks that the server stays available under load:

```sh
siege -b -c 25 -t 30S http://127.0.0.1:8080/
```

## Resources

- HTTP methods, status codes, and message format — [RFC 9110](https://www.rfc-editor.org/rfc/rfc9110.html) and [RFC 9112](https://www.rfc-editor.org/rfc/rfc9112.html).
- TCP connections, byte streams, and connected sockets — [RFC 9293](https://www.rfc-editor.org/rfc/rfc9293.html) and [accept(2)](https://man7.org/linux/man-pages/man2/accept.2.html).
- OSI layers and their roles — [ITU-T X.200, section 7](https://www.itu.int/rec/T-REC-X.200-199407-I/en).
- Event-driven I/O on Linux — [epoll(7)](https://man7.org/linux/man-pages/man7/epoll.7.html).
- Server and location blocks — [NGINX Beginner’s Guide](https://nginx.org/en/docs/beginners_guide.html), plus the [`ngx_http_core_module` directives](https://nginx.org/en/docs/http/ngx_http_core_module.html) (`root`, `alias`, `limit_except`, `error_page`, `client_max_body_size`) and [`autoindex`](https://nginx.org/en/docs/http/ngx_http_autoindex_module.html).
- Communication between a server and a CGI script — [RFC 3875](https://www.rfc-editor.org/rfc/rfc3875.html).
- File uploads with `multipart/form-data` — [RFC 7578](https://www.rfc-editor.org/rfc/rfc7578.html).
- Cookies and session state — [RFC 6265](https://www.rfc-editor.org/rfc/rfc6265.html).

### AI use

AI tools (e.g. Claude, Gemini) were used for the following tasks, on these parts of the project:

- **Concept clarification and summarizing reference material**: understanding the RFCs, `epoll`, and CGI before implementing them. No server code was produced this way.
- **Test scripts**: generating some of the request tests in [`scripts/`](scripts/) (e.g. the chunked, delayed, and incomplete request tests).
- **Debugging review**: reviewing our code and logs to help locate bugs, which we then fixed ourselves.
- **Web resources**: the HTML, CSS, and JS files of the demo sites in [`showcase/www/`](showcase/www/) and [`showcase/www-b/`](showcase/www-b/).

The server itself (config parser, event loop, HTTP parsing, routing, CGI, and sessions) was written by us.
