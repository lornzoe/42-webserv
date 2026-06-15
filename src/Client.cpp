/*
** Client.cpp
** ----------
** Handles one client connection: reading, request dispatching, and writing.
**
** The poll() loop in Server.cpp calls:
**   doRead()  when the socket is readable (POLLIN)
**   doWrite() when the socket is writable (POLLOUT)
**
** Neither of these blocks — they do one round of I/O and return.
**
** Request dispatching (buildResponse / handleRequest):
**   1. Find the matching location block for the request URI.
**   2. Check if the HTTP method is allowed.
**   3. Handle redirects, CGI, static files, uploads, and directory listing.
*/

#include "Client.hpp"
#include "Utils.hpp"
#include <sys/socket.h>   // recv, send
#include <sys/stat.h>     // stat
#include <dirent.h>       // opendir, readdir
#include <unistd.h>       // close, fork, pipe, dup2, execve, access
#include <fcntl.h>        // open, O_CREAT, O_WRONLY
#include <sys/wait.h>     // waitpid
#include <cstring>        // memset, strerror
#include <cerrno>
#include <cstdlib>        // getenv
#include <sstream>
#include <fstream>        // ofstream for uploads
#include <ctime>

// Size of the read buffer per recv() call
static const int READ_BUFFER_SIZE = 8192;

Client::Client(int fd, const ServerConfig* cfg)
    : fd(fd), state(CLIENT_READING), writeOffset(0),
      lastActivity(std::time(NULL)), serverConfig(cfg) {}

// ---------------------------------------------------------------------------
// doRead — called when poll() says socket is readable
// ---------------------------------------------------------------------------
bool Client::doRead() {
    char buf[READ_BUFFER_SIZE];
    // recv() is non-blocking (socket set O_NONBLOCK by Server)
    ssize_t n = recv(fd, buf, sizeof(buf), 0);

    if (n == 0) {
        // Client closed the connection gracefully
        return false;
    }
    if (n < 0) {
        // recv() would block — not an error in non-blocking mode
        // (errno == EAGAIN or EWOULDBLOCK)
        return true;
    }

    lastActivity = std::time(NULL);

    // Feed bytes to the request parser
    request.feed(buf, static_cast<size_t>(n));

    if (request.hasError()) {
        // Malformed request — send 400 and close
        HttpResponse res = HttpResponse::makeError(400);
        writeBuffer = res.toString();
        state = CLIENT_WRITING;
        return true;
    }

    // Check body size against the server limit
    if (request.body.size() > serverConfig->clientMaxBodySize) {
        HttpResponse res = HttpResponse::makeError(413);
        writeBuffer = res.toString();
        state = CLIENT_WRITING;
        return true;
    }

    if (request.isComplete()) {
        // Full request received — build the response
        buildResponse();
        state = CLIENT_WRITING;
    }
    return true;
}

// ---------------------------------------------------------------------------
// doWrite — called when poll() says socket is writable
// ---------------------------------------------------------------------------
bool Client::doWrite() {
    if (writeBuffer.empty() || writeOffset >= writeBuffer.size()) {
        // Nothing to send
        state = CLIENT_DONE;
        return true;
    }

    // Send as many bytes as the socket will accept right now
    ssize_t n = send(fd,
                     writeBuffer.c_str() + writeOffset,
                     writeBuffer.size() - writeOffset,
                     0);
    if (n < 0) {
        // send() would block — try again next poll() iteration
        return true;
    }
    if (n == 0) {
        return false; // Connection dropped
    }

    writeOffset  += static_cast<size_t>(n);
    lastActivity  = std::time(NULL);

    if (writeOffset >= writeBuffer.size()) {
        // All bytes sent
        state = CLIENT_DONE;
    }
    return true;
}

// ---------------------------------------------------------------------------
// buildResponse — turn the parsed request into a serialised response
// ---------------------------------------------------------------------------
void Client::buildResponse() {
    HttpResponse res = handleRequest();
    writeBuffer  = res.toString();
    writeOffset  = 0;
}

// ---------------------------------------------------------------------------
// handleRequest — main dispatcher
// ---------------------------------------------------------------------------
HttpResponse Client::handleRequest() {
    // Only handle HTTP/1.0 and HTTP/1.1
    if (request.httpVersion != "HTTP/1.1" && request.httpVersion != "HTTP/1.0")
        return HttpResponse::makeError(505); // HTTP Version Not Supported

    // Find the matching location block (longest prefix wins)
    const LocationConfig* loc = matchLocation();
    if (!loc)
        return HttpResponse::makeError(404);

    // If the location has a redirect configured, issue it immediately
    if (!loc->redirect.empty())
        return HttpResponse::makeRedirect(loc->redirect, 301);

    // Check whether this method is allowed for this location
    if (!loc->allowedMethods.empty()) {
        bool allowed = false;
        for (size_t i = 0; i < loc->allowedMethods.size(); ++i) {
            if (loc->allowedMethods[i] == request.method) {
                allowed = true;
                break;
            }
        }
        if (!allowed)
            return HttpResponse::makeError(405); // Method Not Allowed
    }

    // Resolve the URI to an absolute filesystem path
    std::string filePath = resolvePath(*loc, request.path);

    // Dispatch by HTTP method
    if (request.method == "GET" || request.method == "HEAD")
        return handleGet(*loc, filePath);
    if (request.method == "POST")
        return handlePost(*loc, filePath);
    if (request.method == "DELETE") {
        // For upload locations, DELETE resolves against uploadDir, not root.
        // e.g. DELETE /upload/foo.txt -> www/uploads/foo.txt
        if (!loc->uploadDir.empty()) {
            std::string rel = request.path;
            if (rel.substr(0, loc->path.size()) == loc->path)
                rel = rel.substr(loc->path.size());
            if (!rel.empty() && rel[0] == '/') rel.erase(0, 1);
            return handleDelete(Utils::joinPath(loc->uploadDir, rel));
        }
        return handleDelete(filePath);
    }

    return HttpResponse::makeError(501); // Not Implemented
}

// ---------------------------------------------------------------------------
// handleGet — serve a file or directory listing
// ---------------------------------------------------------------------------
HttpResponse Client::handleGet(const LocationConfig& loc,
                               const std::string& filePath) {
    // If the path is a directory, we either serve an index file
    // or show a directory listing.
    if (Utils::isDirectory(filePath)) {
        // Try the configured index file first
        if (!loc.index.empty()) {
            std::string indexPath = Utils::joinPath(filePath, loc.index);
            if (Utils::isFile(indexPath)) {
                // Check for CGI on the index file
                if (loc.cgiEnabled &&
                    Utils::fileExtension(indexPath) == loc.cgiExtension) {
                    return executeCgi(loc, indexPath);
                }
                std::string content;
                if (!Utils::readFile(indexPath, content))
                    return HttpResponse::makeError(403);
                HttpResponse res;
                res.setStatus(200);
                res.setBody(content, HttpResponse::mimeType(indexPath));
                return res;
            }
        }
        // No index file — show directory listing if enabled
        if (loc.autoindex)
            return generateDirListing(request.path, filePath);
        return HttpResponse::makeError(403);
    }

    // It's a regular file — check for CGI
    if (loc.cgiEnabled &&
        !loc.cgiExtension.empty() &&
        Utils::fileExtension(filePath) == loc.cgiExtension) {
        return executeCgi(loc, filePath);
    }

    // Serve the static file
    if (!Utils::isFile(filePath))
        return HttpResponse::makeError(404);

    std::string content;
    if (!Utils::readFile(filePath, content))
        return HttpResponse::makeError(403); // Can't read = Forbidden

    HttpResponse res;
    res.setStatus(200);
    res.setBody(content, HttpResponse::mimeType(filePath));
    return res;
}

// ---------------------------------------------------------------------------
// handlePost — file upload or CGI execution
// ---------------------------------------------------------------------------
HttpResponse Client::handlePost(const LocationConfig& loc,
                                const std::string& filePath) {
    // If CGI is configured, run the script with the POST body as stdin
    if (loc.cgiEnabled &&
        !loc.cgiExtension.empty() &&
        Utils::fileExtension(filePath) == loc.cgiExtension) {
        return executeCgi(loc, filePath);
    }

    // File upload: only if the location has an upload_dir configured
    if (!loc.uploadDir.empty()) {
        // Extract filename from Content-Disposition header if present,
        // otherwise use a timestamp-based name
        std::string filename = "upload_" + Utils::toString(std::time(NULL));
        std::map<std::string,std::string>::const_iterator it =
            request.headers.find("content-disposition");
        if (it != request.headers.end()) {
            // Look for filename="..."
            size_t fn = it->second.find("filename=\"");
            if (fn != std::string::npos) {
                fn += 10; // skip past filename="
                size_t fe = it->second.find('"', fn);
                if (fe != std::string::npos)
                    filename = it->second.substr(fn, fe - fn);
            }
        }

        std::string destPath = Utils::joinPath(loc.uploadDir, filename);

        // Write body bytes to the destination file
        std::ofstream out(destPath.c_str(), std::ios::binary);
        if (!out.is_open())
            return HttpResponse::makeError(500);
        out.write(request.body.c_str(), static_cast<std::streamsize>(request.body.size()));
        if (!out)
            return HttpResponse::makeError(500);

        HttpResponse res;
        res.setStatus(201); // 201 Created
        res.setBody("File uploaded successfully.", "text/plain");
        return res;
    }

    // POST to a non-CGI, non-upload location: treat as an error
    return HttpResponse::makeError(405);
}

// ---------------------------------------------------------------------------
// handleDelete — remove a file
// ---------------------------------------------------------------------------
HttpResponse Client::handleDelete(const std::string& filePath) {
    if (!Utils::isFile(filePath))
        return HttpResponse::makeError(404);

    if (::unlink(filePath.c_str()) != 0)
        return HttpResponse::makeError(403); // Can't delete = Forbidden

    HttpResponse res;
    res.setStatus(204); // 204 No Content — successful delete with no body
    return res;
}

// ---------------------------------------------------------------------------
// generateDirListing — build an HTML directory index page
// ---------------------------------------------------------------------------
HttpResponse Client::generateDirListing(const std::string& uriPath,
                                        const std::string& fsPath) {
    DIR* dir = ::opendir(fsPath.c_str());
    if (!dir)
        return HttpResponse::makeError(403);

    std::ostringstream html;
    html << "<!DOCTYPE html><html><head>"
         << "<title>Index of " << uriPath << "</title>"
         << "<style>body{font-family:monospace;} a{display:block;margin:4px 0;}</style>"
         << "</head><body>"
         << "<h1>Index of " << uriPath << "</h1><hr>";

    // Add a parent directory link (unless we are at root)
    if (uriPath != "/") {
        size_t slash = uriPath.rfind('/', uriPath.size() - 2);
        std::string parent = (slash == std::string::npos)
                             ? "/" : uriPath.substr(0, slash + 1);
        html << "<a href=\"" << parent << "\">..</a>";
    }

    struct dirent* entry;
    while ((entry = ::readdir(dir)) != NULL) {
        std::string name = entry->d_name;
        if (name == "." || name == "..") continue;

        // Append a trailing slash for directories
        std::string fullPath = Utils::joinPath(fsPath, name);
        if (Utils::isDirectory(fullPath)) name += "/";

        // Make sure the URI path ends with a slash
        std::string href = uriPath;
        if (!href.empty() && href[href.size()-1] != '/') href += "/";
        href += entry->d_name;
        if (Utils::isDirectory(fullPath)) href += "/";

        html << "<a href=\"" << href << "\">" << name << "</a>";
    }
    ::closedir(dir);

    html << "<hr></body></html>";

    HttpResponse res;
    res.setStatus(200);
    res.setBody(html.str(), "text/html");
    return res;
}

// ---------------------------------------------------------------------------
// executeCgi — fork/exec the CGI script and capture its output
// ---------------------------------------------------------------------------
HttpResponse Client::executeCgi(const LocationConfig& loc,
                                const std::string& scriptPath) {
    // CGI communication:
    //   stdinPipe[0]  = read end   (CGI reads request body from here)
    //   stdinPipe[1]  = write end  (we write request body here)
    //   stdoutPipe[0] = read end   (we read CGI output from here)
    //   stdoutPipe[1] = write end  (CGI writes its response here)
    int stdinPipe[2], stdoutPipe[2];
    if (::pipe(stdinPipe) < 0 || ::pipe(stdoutPipe) < 0)
        return HttpResponse::makeError(500);

    pid_t pid = ::fork();
    if (pid < 0)
        return HttpResponse::makeError(500);

    if (pid == 0) {
        // ---- Child process (the CGI script) --------------------------------
        // Redirect stdin to stdinPipe and stdout to stdoutPipe
        ::dup2(stdinPipe[0],  STDIN_FILENO);
        ::dup2(stdoutPipe[1], STDOUT_FILENO);

        // Close ends we no longer need in the child
        ::close(stdinPipe[1]);
        ::close(stdoutPipe[0]);
        ::close(stdinPipe[0]);
        ::close(stdoutPipe[1]);

        // Change into the directory containing the script so relative paths work.
        // We split scriptPath into dir + basename BEFORE chdir so we can pass
        // only the basename to execve (which runs relative to the new cwd).
        std::string scriptDir;
        std::string scriptBase;
        {
            size_t slash = scriptPath.rfind('/');
            if (slash != std::string::npos) {
                scriptDir  = scriptPath.substr(0, slash);
                scriptBase = scriptPath.substr(slash + 1);
            } else {
                scriptDir  = ".";
                scriptBase = scriptPath;
            }
        }
        if (::chdir(scriptDir.c_str()) != 0)
            ::_exit(1);

        // Build the CGI environment variables as required by RFC 3875
        std::vector<std::string> envStrings;
        envStrings.push_back("REQUEST_METHOD=" + request.method);
        envStrings.push_back("SCRIPT_FILENAME=" + scriptBase);
        envStrings.push_back("SCRIPT_NAME=" + request.path);
        envStrings.push_back("PATH_INFO=" + request.path);
        envStrings.push_back("QUERY_STRING=" + request.queryString);
        envStrings.push_back("CONTENT_LENGTH=" +
            Utils::toString(request.body.size()));

        // Pass Content-Type header to CGI
        std::map<std::string,std::string>::const_iterator it =
            request.headers.find("content-type");
        if (it != request.headers.end())
            envStrings.push_back("CONTENT_TYPE=" + it->second);

        // Pass HTTP_ prefixed headers
        for (it = request.headers.begin(); it != request.headers.end(); ++it) {
            std::string key = "HTTP_";
            for (size_t i = 0; i < it->first.size(); ++i)
                key += static_cast<char>(
                    it->first[i] == '-' ? '_' :
                    std::toupper(static_cast<unsigned char>(it->first[i])));
            envStrings.push_back(key + "=" + it->second);
        }

        // Convert to C-style char* array (execve needs this)
        std::vector<char*> env;
        for (size_t i = 0; i < envStrings.size(); ++i)
            env.push_back(const_cast<char*>(envStrings[i].c_str()));
        env.push_back(NULL);

        // argv: [interpreter, scriptBase, NULL]
        // scriptBase is relative to the chdir'd working directory
        std::vector<char*> argv;
        argv.push_back(const_cast<char*>(loc.cgiPath.c_str()));
        argv.push_back(const_cast<char*>(scriptBase.c_str()));
        argv.push_back(NULL);

        ::execve(loc.cgiPath.c_str(), argv.data(), env.data());
        // If execve returns, something went wrong
        ::_exit(1);
    }

    // ---- Parent process (the server) ---------------------------------------
    // Close ends the parent doesn't use
    ::close(stdinPipe[0]);
    ::close(stdoutPipe[1]);

    // Write the request body to the CGI's stdin
    if (!request.body.empty()) {
        ::write(stdinPipe[1], request.body.c_str(),
                request.body.size());
    }
    ::close(stdinPipe[1]); // Signal EOF to the CGI

    // Read the CGI's output (headers + body)
    // We collect everything then wait for the child to exit
    std::string cgiOutput;
    char buf[4096];
    ssize_t n;
    while ((n = ::read(stdoutPipe[0], buf, sizeof(buf))) > 0)
        cgiOutput.append(buf, static_cast<size_t>(n));
    ::close(stdoutPipe[0]);

    // Reap the child (avoid zombies)
    int status;
    ::waitpid(pid, &status, 0);

    if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
        return HttpResponse::makeError(502); // Bad Gateway

    // Parse the CGI output: it starts with headers, then \r\n\r\n, then body
    // CGI headers look like:
    //   Content-Type: text/html\r\n
    //   Status: 200 OK\r\n      <- optional
    //   \r\n
    //   <html>...
    size_t headerEnd = cgiOutput.find("\r\n\r\n");
    if (headerEnd == std::string::npos)
        headerEnd = cgiOutput.find("\n\n");
    if (headerEnd == std::string::npos)
        return HttpResponse::makeError(502);

    std::string cgiHeaders = cgiOutput.substr(0, headerEnd);
    std::string cgiBody    = cgiOutput.substr(
        headerEnd + (cgiOutput[headerEnd+1] == '\r' ? 4 : 2));

    HttpResponse res;
    res.setStatus(200);
    res.body = cgiBody;

    // Parse CGI headers into response headers
    std::istringstream ss(cgiHeaders);
    std::string line;
    while (std::getline(ss, line)) {
        if (!line.empty() && line[line.size()-1] == '\r')
            line.erase(line.size()-1);
        size_t colon = line.find(':');
        if (colon == std::string::npos) continue;
        std::string name  = Utils::trim(line.substr(0, colon));
        std::string value = Utils::trim(line.substr(colon + 1));
        if (Utils::iequals(name, "status")) {
            // "Status: 200 OK"
            res.setStatus(std::atoi(value.c_str()));
        } else {
            res.setHeader(name, value);
        }
    }
    return res;
}

// ---------------------------------------------------------------------------
// resolvePath — map URI to filesystem path using location root
// ---------------------------------------------------------------------------
std::string Client::resolvePath(const LocationConfig& loc,
                                const std::string& uri) const {
    // Strip the location prefix from the URI to get the relative path
    std::string relativePath = uri;
    if (uri.substr(0, loc.path.size()) == loc.path)
        relativePath = uri.substr(loc.path.size());
    if (!relativePath.empty() && relativePath[0] == '/')
        relativePath.erase(0, 1);

    return Utils::joinPath(loc.root, relativePath);
}

// ---------------------------------------------------------------------------
// matchLocation — longest prefix match among the server's location blocks
// ---------------------------------------------------------------------------
const LocationConfig* Client::matchLocation() const {
    const LocationConfig* best = NULL;
    size_t bestLen = 0;

    for (size_t i = 0; i < serverConfig->locations.size(); ++i) {
        const LocationConfig& loc = serverConfig->locations[i];
        // Check if request path starts with this location's prefix
        if (request.path.substr(0, loc.path.size()) == loc.path) {
            if (loc.path.size() > bestLen) {
                bestLen = loc.path.size();
                best    = &loc;
            }
        }
    }
    return best;
}

bool Client::isTimedOut(int seconds) const {
    return std::difftime(std::time(NULL), lastActivity) > seconds;
}
