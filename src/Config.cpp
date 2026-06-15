/*
** Config.cpp
** ----------
** Parses the webserv configuration file.
**
** Format is inspired by NGINX but simplified:
**
**   server {
**       listen 0.0.0.0:8080;
**       server_name mysite.com;
**       root /var/www/html;
**       index index.html;
**       client_max_body_size 10M;
**       error_page 404 /errors/404.html;
**
**       location / {
**           methods GET POST;
**           autoindex off;
**       }
**   }
**
** Parsing strategy:
**   - Read the file line by line.
**   - Strip comments (anything after '#').
**   - Collect non-brace tokens then look for '{' / '}' separately.
**   - Process keyword tokens according to current depth/context BEFORE
**     updating depth for any '{' on the same line.  This avoids the
**     classic bug where "location / {" bumps depth before we can record
**     the location path.
**   - Depth 0 = top-level (ignored).
**   - Depth 1 = inside server { }.
**   - Depth 2 = inside location { }.
*/

#include "Config.hpp"
#include "Utils.hpp"
#include <fstream>
#include <stdexcept>
#include <cstdlib>   // atoi
#include <cctype>    // toupper

// Parse a size string like "10M", "512K", "1024" into bytes
static size_t parseSize(const std::string& s) {
    if (s.empty()) return 0;
    size_t n = static_cast<size_t>(std::atoi(s.c_str()));
    char suffix = s[s.size() - 1];
    if (suffix == 'M' || suffix == 'm') n *= 1024 * 1024;
    else if (suffix == 'K' || suffix == 'k') n *= 1024;
    return n;
}

// Remove the trailing semicolon (if any) from a token
static std::string stripSemicolon(const std::string& s) {
    if (!s.empty() && s[s.size() - 1] == ';')
        return s.substr(0, s.size() - 1);
    return s;
}

// Split a raw line into tokens on any whitespace, filtering empty strings
static std::vector<std::string> tokenise(const std::string& line) {
    std::vector<std::string> result;
    std::string cur;
    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        if (c == ' ' || c == '\t' || c == '\r') {
            if (!cur.empty()) { result.push_back(cur); cur.clear(); }
        } else {
            cur += c;
        }
    }
    if (!cur.empty()) result.push_back(cur);
    return result;
}

void Config::parse(const std::string& path) {
    std::ifstream file(path.c_str());
    if (!file.is_open())
        throw std::runtime_error("Cannot open config file: " + path);

    int            depth      = 0;
    ServerConfig   curServer;
    LocationConfig curLoc;
    bool           inServer   = false;
    bool           inLocation = false;
    std::string    line;

    while (std::getline(file, line)) {
        // Strip inline comment
        size_t comment = line.find('#');
        if (comment != std::string::npos)
            line = line.substr(0, comment);

        std::vector<std::string> tokens = tokenise(line);
        if (tokens.empty()) continue;

        // ---------------------------------------------------------------
        // Walk through each token on the line in order.
        // We handle directive tokens first (they may appear before '{' on
        // the same line), then adjust depth when we see '{' or '}'.
        // ---------------------------------------------------------------
        for (size_t i = 0; i < tokens.size(); ++i) {
            const std::string& tok = tokens[i];

            // ----- Opening brace -------------------------------------------
            if (tok == "{") {
                ++depth;
                // depth just became 2: we're opening a location block.
                // The location path was recorded when we saw "location" below.
                if (depth == 2) inLocation = true;
                continue;
            }

            // ----- Closing brace -------------------------------------------
            if (tok == "}") {
                if (depth == 2 && inLocation) {
                    // Finalise location: ensure it has at least server's root
                    if (curLoc.root.empty())
                        curLoc.root = curServer.root;
                    if (curLoc.index.empty())
                        curLoc.index = curServer.index;
                    curServer.locations.push_back(curLoc);
                    curLoc       = LocationConfig();
                    inLocation   = false;
                } else if (depth == 1 && inServer) {
                    servers.push_back(curServer);
                    curServer  = ServerConfig();
                    inServer   = false;
                }
                --depth;
                continue;
            }

            // ----- Directives: only make sense as the first token ----------
            if (i != 0) continue;   // value tokens; already consumed by key below

            // ---------------------------------------------------------------
            // Depth 0: only "server" keyword
            // ---------------------------------------------------------------
            if (depth == 0) {
                if (tok == "server") {
                    inServer = true;
                    // Opening brace may be on this line (handled above in
                    // the next iteration) or on the next line — depth is
                    // bumped when we see '{'.
                }
                continue;
            }

            // ---------------------------------------------------------------
            // Depth 1: server-level directives
            // ---------------------------------------------------------------
            if (depth == 1 && inServer) {
                if (tok == "listen" && i + 1 < tokens.size()) {
                    std::string val = stripSemicolon(tokens[++i]);
                    size_t colon = val.find(':');
                    if (colon != std::string::npos) {
                        curServer.host = val.substr(0, colon);
                        curServer.port = std::atoi(val.substr(colon + 1).c_str());
                    } else {
                        curServer.port = std::atoi(val.c_str());
                    }

                } else if (tok == "server_name" && i + 1 < tokens.size()) {
                    curServer.serverName = stripSemicolon(tokens[++i]);

                } else if (tok == "root" && i + 1 < tokens.size()) {
                    curServer.root = stripSemicolon(tokens[++i]);

                } else if (tok == "index" && i + 1 < tokens.size()) {
                    curServer.index = stripSemicolon(tokens[++i]);

                } else if (tok == "client_max_body_size" && i + 1 < tokens.size()) {
                    curServer.clientMaxBodySize = parseSize(stripSemicolon(tokens[++i]));

                } else if (tok == "error_page" && i + 2 < tokens.size()) {
                    int code = std::atoi(tokens[i + 1].c_str());
                    curServer.errorPages[code] = stripSemicolon(tokens[i + 2]);
                    i += 2;

                } else if (tok == "location" && i + 1 < tokens.size()) {
                    // Record path; '{' will be seen later on same line or next
                    curLoc       = LocationConfig();
                    curLoc.path  = stripSemicolon(tokens[++i]);
                    // Pre-inherit from server so location overrides are optional
                    curLoc.root  = curServer.root;
                    curLoc.index = curServer.index;
                }
                continue;
            }

            // ---------------------------------------------------------------
            // Depth 2: location-level directives
            // ---------------------------------------------------------------
            if (depth == 2 && inLocation) {
                if (tok == "methods" || tok == "limit_except") {
                    // Consume all remaining method tokens on the line
                    for (size_t j = i + 1; j < tokens.size(); ++j) {
                        std::string m = stripSemicolon(tokens[j]);
                        // Convert to upper-case
                        for (size_t k = 0; k < m.size(); ++k)
                            m[k] = static_cast<char>(
                                std::toupper(static_cast<unsigned char>(m[k])));
                        if (!m.empty() && m != "{" && m != "}")
                            curLoc.allowedMethods.push_back(m);
                    }
                    i = tokens.size(); // consumed rest of line

                } else if (tok == "root" && i + 1 < tokens.size()) {
                    curLoc.root = stripSemicolon(tokens[++i]);

                } else if (tok == "index" && i + 1 < tokens.size()) {
                    curLoc.index = stripSemicolon(tokens[++i]);

                } else if (tok == "autoindex" && i + 1 < tokens.size()) {
                    curLoc.autoindex = (stripSemicolon(tokens[++i]) == "on");

                } else if (tok == "upload_dir" && i + 1 < tokens.size()) {
                    curLoc.uploadDir = stripSemicolon(tokens[++i]);

                } else if (tok == "return" && i + 1 < tokens.size()) {
                    // "return 301 /url;" or "return /url;"
                    std::string next = stripSemicolon(tokens[i + 1]);
                    // If next looks like a number it's the status code
                    if (!next.empty() && std::isdigit(static_cast<unsigned char>(next[0]))) {
                        if (i + 2 < tokens.size())
                            curLoc.redirect = stripSemicolon(tokens[i + 2]);
                        i += 2;
                    } else {
                        curLoc.redirect = next;
                        ++i;
                    }

                } else if (tok == "cgi_extension" && i + 1 < tokens.size()) {
                    curLoc.cgiEnabled   = true;
                    curLoc.cgiExtension = stripSemicolon(tokens[++i]);

                } else if (tok == "cgi_path" && i + 1 < tokens.size()) {
                    curLoc.cgiPath = stripSemicolon(tokens[++i]);
                }
                continue;
            }
        }
    }

    if (servers.empty())
        throw std::runtime_error("Config file has no valid server blocks");
}
