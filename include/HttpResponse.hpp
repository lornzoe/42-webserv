#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

/*
** HttpResponse.hpp
** ----------------
** Builds and serialises HTTP responses to send back to clients.
**
** A response looks like:
**
**   HTTP/1.1 200 OK\r\n
**   Content-Type: text/html\r\n
**   Content-Length: 42\r\n
**   Connection: close\r\n
**   \r\n
**   <body bytes>
**
** Usage:
**   HttpResponse res;
**   res.setStatus(200);
**   res.setHeader("Content-Type", "text/html");
**   res.setBody("<html>Hello</html>");
**   std::string raw = res.toString();  // then send() this
*/

#include <string>
#include <map>

class HttpResponse {
public:
    int         statusCode;
    std::string statusMessage;
    std::map<std::string, std::string> headers;
    std::string body;

    HttpResponse();

    void setStatus(int code);
    void setHeader(const std::string& name, const std::string& value);
    void setBody(const std::string& content, const std::string& contentType = "text/html");

    // Serialise to a raw byte string ready to send over the socket.
    // Automatically adds Content-Length and Date headers.
    std::string toString() const;

    // Convenience: build a complete error response with a simple HTML body.
    static HttpResponse makeError(int code, const std::string& customPage = "");

    // Convenience: redirect response (301 or 302)
    static HttpResponse makeRedirect(const std::string& location, int code = 301);

    // Map status code to its standard reason phrase
    static std::string reasonPhrase(int code);

    // Guess MIME type from file extension
    static std::string mimeType(const std::string& path);

    // RFC 1123 date string for the Date header
    static std::string httpDate();
};

#endif // HTTPRESPONSE_HPP
