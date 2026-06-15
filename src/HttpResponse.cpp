/*
** HttpResponse.cpp
** ----------------
** Builds HTTP/1.1 responses and serialises them to a raw byte string.
**
** A minimal valid response:
**
**   HTTP/1.1 200 OK\r\n
**   Content-Type: text/html; charset=UTF-8\r\n
**   Content-Length: 13\r\n
**   Connection: close\r\n
**   \r\n
**   Hello, World!
*/

#include "HttpResponse.hpp"
#include "Utils.hpp"
#include <ctime>
#include <sstream>
#include <map>

HttpResponse::HttpResponse() : statusCode(200), statusMessage("OK") {}

void HttpResponse::setStatus(int code) {
    statusCode    = code;
    statusMessage = reasonPhrase(code);
}

void HttpResponse::setHeader(const std::string& name, const std::string& value) {
    headers[name] = value;
}

void HttpResponse::setBody(const std::string& content,
                           const std::string& contentType) {
    body = content;
    setHeader("Content-Type", contentType + "; charset=UTF-8");
}

// Serialise the response into a single string for sending
std::string HttpResponse::toString() const {
    std::ostringstream ss;

    // Status line
    ss << "HTTP/1.1 " << statusCode << " " << statusMessage << "\r\n";

    // Mandatory headers
    ss << "Date: " << httpDate() << "\r\n";
    ss << "Server: webserv/1.0\r\n";
    ss << "Content-Length: " << body.size() << "\r\n";

    // We tell clients not to reuse connections (simplifies our server)
    if (headers.find("Connection") == headers.end())
        ss << "Connection: close\r\n";

    // All other headers the caller set
    for (std::map<std::string,std::string>::const_iterator it = headers.begin();
         it != headers.end(); ++it) {
        ss << it->first << ": " << it->second << "\r\n";
    }

    ss << "\r\n";     // Blank line separates headers from body
    ss << body;
    return ss.str();
}

// Build a self-contained error response page
HttpResponse HttpResponse::makeError(int code, const std::string& customPage) {
    HttpResponse res;
    res.setStatus(code);

    if (!customPage.empty()) {
        // Use the custom error page provided by the config
        res.setBody(customPage, "text/html");
    } else {
        // Generate a minimal default error page
        std::ostringstream html;
        html << "<!DOCTYPE html><html><head><title>"
             << code << " " << reasonPhrase(code)
             << "</title></head><body><h1>"
             << code << " " << reasonPhrase(code)
             << "</h1></body></html>";
        res.setBody(html.str(), "text/html");
    }
    return res;
}

// Build a redirect response
HttpResponse HttpResponse::makeRedirect(const std::string& location, int code) {
    HttpResponse res;
    res.setStatus(code);
    res.setHeader("Location", location);
    std::ostringstream html;
    html << "<!DOCTYPE html><html><body>Redirecting to <a href=\""
         << location << "\">" << location << "</a></body></html>";
    res.setBody(html.str(), "text/html");
    return res;
}

// Standard reason phrases from RFC 7231
std::string HttpResponse::reasonPhrase(int code) {
    switch (code) {
        case 200: return "OK";
        case 201: return "Created";
        case 204: return "No Content";
        case 301: return "Moved Permanently";
        case 302: return "Found";
        case 400: return "Bad Request";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 408: return "Request Timeout";
        case 411: return "Length Required";
        case 413: return "Payload Too Large";
        case 500: return "Internal Server Error";
        case 501: return "Not Implemented";
        case 502: return "Bad Gateway";
        case 504: return "Gateway Timeout";
        default:  return "Unknown";
    }
}

// MIME type lookup by file extension
std::string HttpResponse::mimeType(const std::string& path) {
    // Build a static map the first time this is called
    static std::map<std::string, std::string> types;
    if (types.empty()) {
        types[".html"] = "text/html";
        types[".htm"]  = "text/html";
        types[".css"]  = "text/css";
        types[".js"]   = "application/javascript";
        types[".json"] = "application/json";
        types[".xml"]  = "application/xml";
        types[".txt"]  = "text/plain";
        types[".png"]  = "image/png";
        types[".jpg"]  = "image/jpeg";
        types[".jpeg"] = "image/jpeg";
        types[".gif"]  = "image/gif";
        types[".ico"]  = "image/x-icon";
        types[".svg"]  = "image/svg+xml";
        types[".pdf"]  = "application/pdf";
        types[".zip"]  = "application/zip";
        types[".gz"]   = "application/gzip";
    }
    std::string ext = Utils::fileExtension(path);
    // Convert to lowercase for comparison
    for (size_t i = 0; i < ext.size(); ++i)
        ext[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(ext[i])));
    std::map<std::string,std::string>::iterator it = types.find(ext);
    if (it != types.end()) return it->second;
    return "application/octet-stream"; // Safe default for unknown types
}

// RFC 1123 date format: "Mon, 01 Jan 2024 00:00:00 GMT"
std::string HttpResponse::httpDate() {
    time_t now = std::time(NULL);
    struct tm* gmt = std::gmtime(&now);
    char buf[64];
    // strftime with RFC 1123 format
    std::strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", gmt);
    return std::string(buf);
}
