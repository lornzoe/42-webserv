#ifndef SERVER_HPP
#define SERVER_HPP

/*
** Server.hpp
** ----------
** The core event loop. Sets up listening sockets, then runs a single
** poll() call that multiplexes all I/O across every client connection.
**
** Architecture overview:
**
**   main() -> Server::init() -> Server::run()
**
**   init():
**     For each ServerConfig, bind a TCP socket and call listen().
**     Set sockets to non-blocking mode (O_NONBLOCK via fcntl on macOS,
**     or via socket flags on Linux).
**
**   run():
**     Build a pollfd array containing:
**       - all listening sockets (POLLIN only — we're waiting for new clients)
**       - all client sockets   (POLLIN if reading, POLLOUT if writing)
**     Call poll() with a timeout.
**     On return:
**       - New connection on listener -> accept() -> create Client
**       - POLLIN on client  -> Client::doRead()
**       - POLLOUT on client -> Client::doWrite()
**       - Error or timeout  -> clean up client
**
** We never read/write a socket without poll() saying it is ready.
** This satisfies the subject's non-blocking requirement.
*/

#include "Config.hpp"
#include "Client.hpp"
#include <vector>
#include <map>
#include <poll.h>

class Server {
public:
    explicit Server(const Config& cfg);
    ~Server();

    // Create and bind all listening sockets. Call before run().
    void init();

    // Enter the main event loop. Runs until SIGINT/SIGTERM.
    void run();

private:
    const Config&              config_;
    // Map from listening socket fd -> index into config_.servers
    std::map<int, size_t>      listenFds_;
    // Map from client socket fd -> Client object
    std::map<int, Client*>     clients_;

    // Accept a new connection on the given listening socket
    void acceptClient(int listenFd, size_t serverIndex);

    // Remove and close a client connection
    void removeClient(int clientFd);

    // Create a non-blocking TCP socket, bind it, and start listening
    int createListenSocket(const std::string& host, int port);

    // Client timeout in seconds (requests that stall get dropped)
    static const int TIMEOUT_SECONDS = 30;
};

#endif // SERVER_HPP
