#ifndef CONFIG_HPP
#define CONFIG_HPP

/*
** Config.hpp
** ----------
** Holds all parsed configuration data for the server.
** Each "server" block in the config file becomes a ServerConfig.
** Each "location" block inside a server becomes a LocationConfig.
**
** The hierarchy mirrors NGINX's config structure:
**   server {
**       listen 8080;
**       root /var/www;
**       location /api { ... }
**   }
*/

#include <string>
#include <vector>
#include <map>

// ---------------------------------------------------------------------------
// LocationConfig
// Represents one `location /path { ... }` block.
// ---------------------------------------------------------------------------
struct LocationConfig {
    std::string path;               // URL prefix this location matches, e.g. "/api"
    std::string root;               // Filesystem root for this location
    std::string index;              // Default file to serve when URL is a directory
    std::string uploadDir;          // Where uploaded files are saved
    std::string redirect;           // If set, issue HTTP redirect to this URL
    std::vector<std::string> allowedMethods; // e.g. {"GET","POST","DELETE"}
    bool        autoindex;          // Show directory listing when no index file found
    bool        cgiEnabled;         // Whether CGI execution is allowed here
    std::string cgiExtension;       // File extension that triggers CGI, e.g. ".php"
    std::string cgiPath;            // Path to CGI interpreter, e.g. "/usr/bin/php-cgi"

    // Constructor: sane defaults
    LocationConfig()
        : autoindex(false), cgiEnabled(false) {}
};

// ---------------------------------------------------------------------------
// ServerConfig
// Represents one `server { ... }` block — one virtual server.
// ---------------------------------------------------------------------------
struct ServerConfig {
    std::string              host;           // IP to bind, e.g. "0.0.0.0"
    int                      port;           // Port to listen on, e.g. 8080
    std::string              serverName;     // Optional name (for Host: header matching)
    std::string              root;           // Default document root
    std::string              index;          // Default index file
    size_t                   clientMaxBodySize; // Max request body bytes (default 1MB)
    std::map<int,std::string> errorPages;   // Custom error pages: {404 -> "/errors/404.html"}
    std::vector<LocationConfig> locations;  // All location blocks in this server

    // Constructor: sane defaults
    ServerConfig()
        : host("0.0.0.0"), port(8080),
          clientMaxBodySize(1 * 1024 * 1024) {}
};

// ---------------------------------------------------------------------------
// Config
// Top-level object holding all server blocks after parsing.
// ---------------------------------------------------------------------------
class Config {
public:
    std::vector<ServerConfig> servers;

    // Parse the config file at the given path.
    // Throws std::runtime_error on syntax errors.
    void parse(const std::string& path);

private:
    // Helper: find the matching location block for a given URI inside a server
    const LocationConfig* matchLocation(const ServerConfig& srv,
                                        const std::string& uri) const;
};

#endif // CONFIG_HPP
