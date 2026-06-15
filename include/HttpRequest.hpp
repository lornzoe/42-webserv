#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

/*
** HttpRequest.hpp
** ---------------
** Parses raw HTTP/1.1 request bytes received from a client socket.
**
** An HTTP request looks like:
**
**   GET /index.html HTTP/1.1\r\n
**   Host: localhost:8080\r\n
**   Content-Length: 0\r\n
**   \r\n
**   <optional body>
**
** We parse it in two stages:
**   1. Accumulate bytes from recv() into a buffer.
**   2. Once we have the full headers (double CRLF), parse method/URI/headers.
**   3. If Content-Length > 0, keep reading body bytes.
**   4. Support chunked Transfer-Encoding by un-chunking the body.
*/

#include <string>
#include <map>

// Parse states drive our incremental parser so we never block waiting for data
enum ParseState {
    PARSE_REQUEST_LINE, // Still reading the first line (GET /path HTTP/1.1)
    PARSE_HEADERS,      // Reading header lines
    PARSE_BODY,         // Reading body (Content-Length known)
    PARSE_CHUNKED,      // Reading chunked body
    PARSE_COMPLETE,     // Full request received and parsed
    PARSE_ERROR         // Malformed request
};

class HttpRequest {
public:
    // --- Parsed fields -------------------------------------------------------
    std::string method;          // "GET", "POST", "DELETE", etc.
    std::string uri;             // Raw URI, e.g. "/path?query=1"
    std::string path;            // URI path only, e.g. "/path"
    std::string queryString;     // Everything after '?', e.g. "query=1"
    std::string httpVersion;     // "HTTP/1.1"
    std::map<std::string, std::string> headers; // Header name (lowercase) -> value
    std::string body;            // Request body bytes

    // --- Parser state --------------------------------------------------------
    ParseState  state;
    std::string rawBuffer;       // Accumulated bytes not yet parsed
    size_t      contentLength;   // From Content-Length header (0 if absent)
    bool        chunked;         // True when Transfer-Encoding: chunked

    HttpRequest();

    // Feed bytes from recv() into the parser.
    // Returns true when the request is fully parsed (state == PARSE_COMPLETE).
    bool feed(const char* data, size_t len);

    // True once all request data has arrived.
    bool isComplete() const { return state == PARSE_COMPLETE; }
    bool hasError()   const { return state == PARSE_ERROR;    }

    // Decode percent-encoded URI characters (%20 -> ' ', etc.)
    static std::string urlDecode(const std::string& s);

private:
    void parseRequestLine(const std::string& line);
    void parseHeaderLine(const std::string& line);
    void splitUri();             // Separate path from query string
    bool processBody();          // Handle body reading after headers complete
    bool unchunkBody();          // Decode chunked transfer encoding
};

#endif // HTTPREQUEST_HPP
