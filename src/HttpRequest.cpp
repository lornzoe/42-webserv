/*
** HttpRequest.cpp
** ---------------
** Incremental HTTP request parser.
**
** We receive raw bytes from a non-blocking socket and need to handle
** the case where a complete request arrives in multiple recv() calls
** (e.g. large POST bodies) or even multiple requests in one call
** (HTTP pipelining — we don't support that fully but handle it safely).
**
** State machine:
**
**   PARSE_REQUEST_LINE
**     -> Look for "\r\n" to end the first line
**     -> Parse METHOD URI HTTP/x.x
**     -> Move to PARSE_HEADERS
**
**   PARSE_HEADERS
**     -> Look for "\r\n" to end each header line
**     -> Empty line ("\r\n\r\n") ends headers section
**     -> Check Content-Length / Transfer-Encoding
**     -> If body expected: PARSE_BODY or PARSE_CHUNKED
**     -> Otherwise: PARSE_COMPLETE
**
**   PARSE_BODY
**     -> Accumulate bytes until rawBuffer has >= contentLength bytes
**     -> Then: PARSE_COMPLETE
**
**   PARSE_CHUNKED
**     -> Un-chunk the body (see unchunkBody())
**     -> When zero-length chunk found: PARSE_COMPLETE
*/

#include "HttpRequest.hpp"
#include "Utils.hpp"
#include <cctype>
#include <cstdlib>   // strtoul

HttpRequest::HttpRequest()
    : state(PARSE_REQUEST_LINE), contentLength(0), chunked(false) {}

bool HttpRequest::feed(const char* data, size_t len) {
    // Append new bytes to our rolling buffer
    rawBuffer.append(data, len);

    // Keep processing as long as we make progress
    bool progress = true;
    while (progress && state != PARSE_COMPLETE && state != PARSE_ERROR) {
        progress = false;

        if (state == PARSE_REQUEST_LINE || state == PARSE_HEADERS) {
            // Find the next complete line ending with \r\n
            size_t crlf = rawBuffer.find("\r\n");
            if (crlf == std::string::npos)
                break; // Need more data

            std::string line = rawBuffer.substr(0, crlf);
            rawBuffer.erase(0, crlf + 2); // consume line + CRLF
            progress = true;

            if (state == PARSE_REQUEST_LINE) {
                parseRequestLine(line);
                if (state == PARSE_ERROR) break;
                state = PARSE_HEADERS;

            } else { // PARSE_HEADERS
                if (line.empty()) {
                    // Blank line marks end of headers
                    // Determine what to do with the body
                    std::map<std::string,std::string>::iterator it;
                    it = headers.find("transfer-encoding");
                    if (it != headers.end() &&
                        it->second.find("chunked") != std::string::npos) {
                        chunked = true;
                        state = PARSE_CHUNKED;
                    } else {
                        it = headers.find("content-length");
                        if (it != headers.end())
                            contentLength = static_cast<size_t>(
                                std::atoi(it->second.c_str()));
                        if (contentLength > 0)
                            state = PARSE_BODY;
                        else
                            state = PARSE_COMPLETE;
                    }
                } else {
                    parseHeaderLine(line);
                }
            }
        }

        if (state == PARSE_BODY) {
            // Wait until we have all body bytes
            if (rawBuffer.size() >= contentLength) {
                body = rawBuffer.substr(0, contentLength);
                rawBuffer.erase(0, contentLength);
                state = PARSE_COMPLETE;
                progress = true;
            }
        }

        if (state == PARSE_CHUNKED) {
            if (unchunkBody()) {
                state = PARSE_COMPLETE;
                progress = true;
            }
        }
    }
    return state == PARSE_COMPLETE;
}

// Parse "GET /path HTTP/1.1"
void HttpRequest::parseRequestLine(const std::string& line) {
    std::vector<std::string> parts = Utils::split(line, ' ');
    if (parts.size() < 3) {
        state = PARSE_ERROR;
        return;
    }
    method      = parts[0];
    uri         = parts[1];
    httpVersion = parts[2];
    splitUri();
}

// Parse "Header-Name: value"
void HttpRequest::parseHeaderLine(const std::string& line) {
    size_t colon = line.find(':');
    if (colon == std::string::npos) return; // Malformed header — ignore
    std::string name  = Utils::toLower(Utils::trim(line.substr(0, colon)));
    std::string value = Utils::trim(line.substr(colon + 1));
    headers[name] = value;
}

// Split URI "/path?query=1&x=2" into path and queryString
void HttpRequest::splitUri() {
    size_t q = uri.find('?');
    if (q != std::string::npos) {
        path        = uri.substr(0, q);
        queryString = uri.substr(q + 1);
    } else {
        path        = uri;
        queryString = "";
    }
    // Decode percent-encoded characters in the path
    path = urlDecode(path);
}

// Decode %XX sequences and '+' (as space) in URI-encoded strings
std::string HttpRequest::urlDecode(const std::string& s) {
    std::string result;
    result.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '%' && i + 2 < s.size() &&
            std::isxdigit(static_cast<unsigned char>(s[i+1])) &&
            std::isxdigit(static_cast<unsigned char>(s[i+2]))) {
            // Convert two hex digits to a byte
            char hex[3] = { s[i+1], s[i+2], 0 };
            result += static_cast<char>(std::strtoul(hex, NULL, 16));
            i += 2;
        } else if (s[i] == '+') {
            result += ' ';
        } else {
            result += s[i];
        }
    }
    return result;
}

/*
** Un-chunk a chunked Transfer-Encoding body.
**
** Chunked format:
**   <hex size>\r\n
**   <data bytes>\r\n
**   <hex size>\r\n
**   <data bytes>\r\n
**   0\r\n            <- zero-size chunk signals end of body
**   \r\n             <- optional trailers followed by empty line
**
** We consume as much of rawBuffer as we can, appending decoded
** data to body.  Returns true when the terminal zero-chunk is seen.
*/
bool HttpRequest::unchunkBody() {
    while (true) {
        // Find the chunk size line
        size_t crlf = rawBuffer.find("\r\n");
        if (crlf == std::string::npos) return false; // Need more data

        // Parse hex chunk size
        std::string sizeLine = rawBuffer.substr(0, crlf);
        // Strip chunk extensions (semicolons after size)
        size_t semi = sizeLine.find(';');
        if (semi != std::string::npos) sizeLine = sizeLine.substr(0, semi);
        sizeLine = Utils::trim(sizeLine);

        size_t chunkSize = static_cast<size_t>(
            std::strtoul(sizeLine.c_str(), NULL, 16));

        if (chunkSize == 0) {
            // Terminal chunk — body is complete
            // Skip the trailing \r\n (and any trailers we ignore)
            rawBuffer.erase(0, crlf + 2);
            return true;
        }

        // Check we have the full chunk data + its trailing \r\n
        size_t needed = crlf + 2 + chunkSize + 2;
        if (rawBuffer.size() < needed) return false; // Need more data

        // Append chunk data to body
        body.append(rawBuffer, crlf + 2, chunkSize);
        // Remove: size line + CRLF + data + CRLF
        rawBuffer.erase(0, needed);
    }
}
