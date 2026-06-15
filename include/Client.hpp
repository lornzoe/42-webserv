#ifndef CLIENT_HPP
#define CLIENT_HPP

/*
** Client.hpp
** ----------
** Tracks the state of one connected client socket.
**
** Because the server uses non-blocking I/O with a single poll() loop,
** we can't just do:
**   recv() -> process -> send()
** in sequence. Instead we track where each client is in the pipeline:
**
**   READING  -> accumulate bytes from recv() until request is complete
**   WRITING  -> send response bytes progressively via send()
**   DONE     -> connection can be closed
**
** The Client object owns the per-connection request, response, and
** write-buffer so the poll loop can resume partial sends/reads at any time.
*/

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "Config.hpp"
#include <string>
#include <ctime>

enum ClientState {
    CLIENT_READING,   // Waiting for / reading request data
    CLIENT_WRITING,   // Sending response data
    CLIENT_DONE       // Done; close the connection
};

class Client {
public:
    int          fd;           // The accepted socket file descriptor
    ClientState  state;        // Current I/O state
    HttpRequest  request;      // Parsed (or in-progress) request
    std::string  writeBuffer;  // Serialised response waiting to be sent
    size_t       writeOffset;  // How many bytes of writeBuffer already sent
    time_t       lastActivity; // Unix timestamp of last I/O — for timeout detection

    // Reference to the server config that owns this client's listening socket
    const ServerConfig* serverConfig;

    explicit Client(int fd, const ServerConfig* cfg);

    // Called by the poll loop when the socket is readable.
    // Returns false if the client disconnected or hit a fatal error.
    bool doRead();

    // Called by the poll loop when the socket is writable.
    // Returns false if send failed and connection should be closed.
    bool doWrite();

    // Build the response for the completed request and store in writeBuffer.
    void buildResponse();

    // Check whether this client has been idle longer than `seconds`
    bool isTimedOut(int seconds) const;

private:
    // Route the completed request to the right handler
    HttpResponse handleRequest();
    HttpResponse handleGet(const LocationConfig& loc, const std::string& filePath);
    HttpResponse handlePost(const LocationConfig& loc, const std::string& filePath);
    HttpResponse handleDelete(const std::string& filePath);

    // Directory listing: generate an HTML page listing directory contents
    HttpResponse generateDirListing(const std::string& uriPath,
                                    const std::string& fsPath);

    // CGI execution: fork + exec the CGI script and capture its output
    HttpResponse executeCgi(const LocationConfig& loc,
                            const std::string& scriptPath);

    // Resolve URI to an absolute filesystem path using location config
    std::string resolvePath(const LocationConfig& loc,
                            const std::string& uri) const;

    // Find the best matching location for the request URI
    const LocationConfig* matchLocation() const;
};

#endif // CLIENT_HPP
