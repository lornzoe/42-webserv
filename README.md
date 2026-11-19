_This project has been created as part of the 42 curriculum by lyanga, ypua, julhong_

# Webserv

## Description

Webserv is an HTTP/1.1 server we built from scratch in C++98 to understand how a browser request becomes a response.

**HTTP** (OSI application layer, layer 7) gives the browser and server a shared language. `GET` asks for a file, `200 OK` signals success, and `Content-Type` tells the browser how to interpret the body. The browser writes its request to a **socket**, its local endpoint for the connection. **TCP** (transport layer, layer 4) carries the bytes in order and retransmits lost data. At the other end, Webserv reads the request and writes the response through its own connected socket.

### What happens when you open `/docs/guide.txt`?

1. **Connect.** When the browser connects to port 8080, `accept()` gives the `Listener` a new socket for that connection. The listening socket stays available for the next browser. In the OSI model, the **session layer (layer 5)** describes how two parties manage an ongoing exchange. In Webserv, the `Server` gives the connected socket to a `Client` object, which tracks reads and writes across successive HTTP requests.
2. **Read.** A single HTTP request can arrive across several reads. `Client` buffers the bytes until the parser can read the request line, headers, and any body. For uploads, it uses `Content-Length` or chunked encoding to find the end of the body.
3. **Route.** The request line `GET /docs/guide.txt HTTP/1.1` asks for a file. The longest matching `location` is `/docs/`, whose configured root maps the URL to `showcase/www/files/docs/guide.txt`.
4. **Reply.** Webserv sends `200 OK`, a `Content-Type: text/plain` header, and the file contents. Ask for a missing file and it sends `404 Not Found` with the site's error page. Both responses follow the same write path, and the server keeps serving other clients.

One server process handles these connections with non-blocking sockets and a single Linux `epoll` loop. `epoll` reports which listeners have connections to accept and which client sockets or CGI pipes are ready to read or write. While one browser pauses halfway through an upload, the loop can send the guide to another. If a write sends only part of a response, the `Client` remembers where to resume.

### Features

- **Configuration.** NGINX-style `server` and `location` blocks define each site's ports and directories, allowed methods, body size limits, redirects, and custom error pages.
- **Files.** `GET` serves static files. For a directory, the server looks for a configured index file and can list the contents if it finds none. On upload routes, `POST` accepts multipart file uploads and `DELETE` removes uploaded files.
- **CGI.** The Common Gateway Interface lets a script generate the response. The showcase maps `.py` to Python and `.pl` to Perl.
- **Sessions.** A `sid` cookie identifies a profile stored in server memory. Save a name on the profile page, then reload to see the name persist and the visit count increase.

## Instructions

Build on Linux with `make` and a C++98 compiler. The showcase also needs executable interpreters at `/usr/bin/python3` and `/usr/bin/perl`. If yours are elsewhere, update the two `cgi` paths in [the showcase configuration](showcase/full.conf).

From the repository root, build and start the showcase.

```sh
make
./webserv showcase/full.conf
```

Open [the showcase](http://localhost:8080/) to try the upload form, CGI demos, and session profile. The same site also listens on port 8081. A [second site](http://localhost:8082/) shows how one process serves different content on another port.

Press `Ctrl+C` to stop the server. Running `./webserv` without an argument loads [the basic configuration](dev/configs/basic.conf). For individual routes and example requests, see the [showcase guide](showcase/README.md).

## Resources

- HTTP methods, status codes, and message format — [RFC 9110](https://www.rfc-editor.org/rfc/rfc9110.html) and [RFC 9112](https://www.rfc-editor.org/rfc/rfc9112.html).
- TCP connections, byte streams, and connected sockets — [RFC 9293](https://www.rfc-editor.org/rfc/rfc9293.html) and [accept(2)](https://man7.org/linux/man-pages/man2/accept.2.html).
- OSI layers and their roles — [ITU-T X.200, section 7](https://www.itu.int/rec/T-REC-X.200-199407-I/en).
- Event-driven I/O on Linux — [epoll(7)](https://man7.org/linux/man-pages/man7/epoll.7.html).
- Server and location blocks — [NGINX Beginner’s Guide](https://nginx.org/en/docs/beginners_guide.html).
- Communication between a server and a CGI script — [RFC 3875](https://www.rfc-editor.org/rfc/rfc3875.html).

### AI use
- Concept clarification
- Summarization of reference material
- Generation of html demonstration pages
- Generation of testing scripts
- Debugging review

Examples include our [HTTP RFC guide](__references/rfc_reference_guide.md), [request playground](showcase/www/playground/index.html), and [test scripts](scripts/).
