/*
** main.cpp
** --------
** Entry point for webserv.
**
** Usage:
**   ./webserv [config_file]
**
** If no config file is given, we try "default.conf" in the current directory.
**
** Flow:
**   1. Parse the configuration file -> Config object
**   2. Create the Server with that Config
**   3. Init (bind sockets)
**   4. Run the event loop until SIGINT/SIGTERM
*/

#include "Config.hpp"
#include "Server.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    // Default configuration file path
    std::string configPath = "config/default.conf";

    if (argc >= 2)
        configPath = argv[1];   // Use the path the user provided
    else if (argc > 2) {
        std::cerr << "Usage: ./webserv [configuration_file]" << std::endl;
        return 1;
    }

    try {
        // Step 1: Parse configuration
        Config config;
        config.parse(configPath);
        std::cout << "[webserv] Loaded config from: " << configPath << std::endl;
        std::cout << "[webserv] " << config.servers.size()
                  << " server block(s) found." << std::endl;

        // Step 2 & 3: Create and initialise server (binds ports)
        Server server(config);
        server.init();

        // Step 4: Run the poll() event loop (blocks until signal)
        server.run();

    } catch (const std::exception& e) {
        std::cerr << "[webserv] Fatal error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
