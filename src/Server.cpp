/*
** Server.cpp
** ----------
** The main event loop using poll() for non-blocking multiplexed I/O.
**
** KEY DESIGN DECISIONS (required by the subject):
**
** 1. SINGLE poll() CALL for all I/O
**    We build one pollfd array containing every fd (listeners + clients)
**    and call poll() once per loop iteration.
**
** 2. NON-BLOCKING sockets
**    All sockets are set to O_NONBLOCK so recv()/send()/accept() never block.
**    If they would block, they return -1 with errno == EAGAIN/EWOULDBLOCK
**    and we simply try again on the next poll() iteration.
**
** 3. poll() monitors BOTH reading AND writing
**    Each client fd has POLLIN set when we want to read from it,
**    and POLLOUT set when we have data to send. This prevents busy-waiting.
**
** 4. We NEVER read or write without poll() saying the fd is ready
**    This is enforced by the structure: only doRead() after POLLIN,
**    only doWrite() after POLLOUT.
**
** 5. errno is NOT used to control server behaviour after read/write
**    (the subject explicitly forbids this).
*/

#include "Server.hpp"
#include <sys/socket.h>    // socket, bind, listen, accept, setsockopt
#include <netinet/in.h>    // sockaddr_in
#include <arpa/inet.h>     // inet_addr
#include <fcntl.h>         // fcntl, F_SETFL, O_NONBLOCK
#include <unistd.h>        // close
#include <cstring>         // memset, strerror
#include <cerrno>
#include <stdexcept>
#include <iostream>
#include <algorithm>       // std::remove
#include "Utils.hpp"

// Allow clean shutdown on SIGINT/SIGTERM
#include <csignal>
static volatile bool g_running = true;
static void handleSignal(int) { g_running = false; }

Server::Server(const Config& cfg) : config_(cfg) {}

Server::~Server() {
    // Close all client connections
    for (std::map<int,Client*>::iterator it = clients_.begin();
         it != clients_.end(); ++it) {
        ::close(it->first);
        delete it->second;
    }
    // Close all listening sockets
    for (std::map<int,size_t>::iterator it = listenFds_.begin();
         it != listenFds_.end(); ++it) {
        ::close(it->first);
    }
}

// ---------------------------------------------------------------------------
// init — create and bind a listening socket for each server block
// ---------------------------------------------------------------------------
void Server::init() {
    std::signal(SIGINT,  handleSignal);
    std::signal(SIGTERM, handleSignal);
    // Ignore SIGPIPE so that writing to a disconnected client doesn't crash us
    std::signal(SIGPIPE, SIG_IGN);

    for (size_t i = 0; i < config_.servers.size(); ++i) {
        const ServerConfig& srv = config_.servers[i];
        int fd = createListenSocket(srv.host, srv.port);
        listenFds_[fd] = i;
        std::cout << "[webserv] Listening on "
                  << srv.host << ":" << srv.port << std::endl;
    }
    if (listenFds_.empty())
        throw std::runtime_error("No listening sockets created");
}

// ---------------------------------------------------------------------------
// run — the main poll() event loop
// ---------------------------------------------------------------------------
void Server::run() {
    std::cout << "[webserv] Server running. Press Ctrl+C to stop." << std::endl;

    while (g_running) {
        // Build the pollfd array.
        // First: all listening sockets (we only care about new connections = POLLIN).
        // Then:  all client sockets (POLLIN if reading, POLLOUT if writing).
        std::vector<struct pollfd> fds;

        for (std::map<int,size_t>::iterator it = listenFds_.begin();
             it != listenFds_.end(); ++it) {
            struct pollfd pfd;
            pfd.fd      = it->first;
            pfd.events  = POLLIN;
            pfd.revents = 0;
            fds.push_back(pfd);
        }

        for (std::map<int,Client*>::iterator it = clients_.begin();
             it != clients_.end(); ++it) {
            Client* client = it->second;
            struct pollfd pfd;
            pfd.fd      = it->first;
            pfd.revents = 0;
            if (client->state == CLIENT_READING)
                pfd.events = POLLIN;
            else if (client->state == CLIENT_WRITING)
                pfd.events = POLLOUT;
            else
                pfd.events = 0;
            fds.push_back(pfd);
        }

        // poll() with a 1-second timeout so we can check for timed-out clients
        int ready = ::poll(fds.data(), static_cast<nfds_t>(fds.size()), 1000);

        if (ready < 0) {
            if (errno == EINTR) continue; // Interrupted by signal — loop again
            std::cerr << "[webserv] poll() error: " << strerror(errno) << std::endl;
            break;
        }

        // Collect file descriptors to remove (we can't modify map while iterating)
        std::vector<int> toRemove;

        // --- Process events ---
        for (size_t i = 0; i < fds.size(); ++i) {
            struct pollfd& pfd = fds[i];
            if (pfd.revents == 0) continue; // No event on this fd

            // Is this a listening socket?
            if (listenFds_.count(pfd.fd)) {
                if (pfd.revents & POLLIN) {
                    // New client connection
                    acceptClient(pfd.fd, listenFds_[pfd.fd]);
                }
                continue;
            }

            // It's a client socket
            Client* client = clients_[pfd.fd];
            if (!client) continue;

            // Check for errors or hang-up first
            if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL)) {
                toRemove.push_back(pfd.fd);
                continue;
            }

            bool ok = true;
            if ((pfd.revents & POLLIN) && client->state == CLIENT_READING)
                ok = client->doRead();
            else if ((pfd.revents & POLLOUT) && client->state == CLIENT_WRITING)
                ok = client->doWrite();

            if (!ok || client->state == CLIENT_DONE)
                toRemove.push_back(pfd.fd);
        }

        // Check for timed-out clients (those that haven't sent a complete
        // request within TIMEOUT_SECONDS)
        for (std::map<int,Client*>::iterator it = clients_.begin();
             it != clients_.end(); ++it) {
            if (it->second->isTimedOut(TIMEOUT_SECONDS)) {
                // Send a 408 Request Timeout if we haven't started writing yet
                if (it->second->state == CLIENT_READING) {
                    HttpResponse res = HttpResponse::makeError(408);
                    it->second->writeBuffer = res.toString();
                    it->second->state = CLIENT_WRITING;
                    // We'll let the normal write path close it
                } else {
                    toRemove.push_back(it->first);
                }
            }
        }

        // Remove closed clients
        for (size_t i = 0; i < toRemove.size(); ++i)
            removeClient(toRemove[i]);
    }
    std::cout << "\n[webserv] Shutting down." << std::endl;
}

// ---------------------------------------------------------------------------
// acceptClient — accept a new TCP connection from a listening socket
// ---------------------------------------------------------------------------
void Server::acceptClient(int listenFd, size_t serverIndex) {
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    int clientFd = ::accept(listenFd,
                            reinterpret_cast<struct sockaddr*>(&addr), &len);
    if (clientFd < 0) {
        // accept() would block or other transient error — skip
        return;
    }

    // Make the client socket non-blocking too
    int flags = ::fcntl(clientFd, F_GETFL, 0);
    if (flags < 0 || ::fcntl(clientFd, F_SETFL, flags | O_NONBLOCK) < 0) {
        ::close(clientFd);
        return;
    }

    // Create a Client object to track this connection
    const ServerConfig* cfg = &config_.servers[serverIndex];
    clients_[clientFd] = new Client(clientFd, cfg);
}

// ---------------------------------------------------------------------------
// removeClient — close a connection and free its resources
// ---------------------------------------------------------------------------
void Server::removeClient(int clientFd) {
    std::map<int,Client*>::iterator it = clients_.find(clientFd);
    if (it == clients_.end()) return;
    ::close(clientFd);
    delete it->second;
    clients_.erase(it);
}

// ---------------------------------------------------------------------------
// createListenSocket — create, configure, bind, and listen on a TCP socket
// ---------------------------------------------------------------------------
int Server::createListenSocket(const std::string& host, int port) {
    // Create a TCP socket (IPv4)
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        throw std::runtime_error(std::string("socket() failed: ") + strerror(errno));

    // Allow the address to be reused immediately after restart
    // (avoids "Address already in use" errors during development)
    int opt = 1;
    if (::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        throw std::runtime_error(std::string("setsockopt() failed: ") + strerror(errno));

    // Set non-blocking mode using fcntl()
    // On macOS this is the approved way; on Linux it's the same
    int flags = ::fcntl(fd, F_GETFL, 0);
    if (flags < 0 || ::fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)
        throw std::runtime_error(std::string("fcntl() O_NONBLOCK failed: ") + strerror(errno));

    // Bind to the specified host and port
    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(static_cast<uint16_t>(port));
    addr.sin_addr.s_addr = (host == "0.0.0.0" || host.empty())
                           ? INADDR_ANY : ::inet_addr(host.c_str());

    if (::bind(fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        ::close(fd);
        throw std::runtime_error(
            std::string("bind() failed on port ") +
            Utils::toString(port) + ": " + strerror(errno));
    }

    // Start listening — backlog of 128 pending connections
    if (::listen(fd, 128) < 0) {
        ::close(fd);
        throw std::runtime_error(std::string("listen() failed: ") + strerror(errno));
    }

    return fd;
}
