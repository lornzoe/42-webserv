_This project has been created as part of the 42 curriculum by lyanga, julhong, ypua._

# Description

## Goal

The goal of the **Webserv** project is to build a functional HTTP server from scratch in C++, while developing a deeper understanding of:

- **HTTP (Hypertext Transfer Protocol)**
- Web servers, such as **NGINX**
- Network programming and client-server communication
- **CGI (Common Gateway Interface)** execution

## Overview

**Webserv** is a custom HTTP/1.1 server implemented in C++.

The server is designed to accept client connections, parse HTTP requests, process them according to the server configuration, and return appropriate HTTP responses.

### Features

- Handle multiple client connections
- Properly handle client disconnections
- Support standard web browsers
- Parse and process HTTP/1.1 requests
- Return appropriate HTTP status codes
- Serve configurable default error pages
- Serve static files
- Handle file uploads
- Handle the following HTTP methods:
  - GET
  - POST
  - DELETE
- Listen on multiple ports
- Serve different content depending on the configured server
- Configure the maximum allowed size for client request bodies
- Parse server configuration files that defining:
  - Listening ports
  - Server names
  - Root directories
  - Index files
  - Allowed HTTP methods
  - Client request body size limits
  - Error pages
  - Upload directories
  - CGI configuration
- Support CGI execution

### Bonus

- Cookie handling
- Session management
- Support for multiple CGI types

# Instructions

## Compilation

```bash
make
```

## Execution

```bash
./webserv config_file
```

# Resources

- [NGINX Beginner's Guide](https://nginx.org/en/docs/beginners_guide.html)
- [RFC 9110 - HTTP Semantics](https://www.rfc-editor.org/info/rfc9110/)
- [RFC 9112 - HTTP/1.1](https://www.rfc-editor.org/info/rfc9112/)
