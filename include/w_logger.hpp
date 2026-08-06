#ifndef LOGGER_H
# define LOGGER_H

// Minutiae
# ifdef DEBUG
#  define LOG_DEBUG(message) std::cerr << "[DEBUG] " << message << std::endl
# else
#  define LOG_DEBUG(message) do {} while(0)
# endif

// Normal operation
#  define LOG_INFO(message) std::cerr << "[INFO] " << message << std::endl

// Error but able to continue
#  define LOG_WARN(message) std::cerr << "[WARNING] " << message << std::endl

// Unable to continue
#  define LOG_ERR(message) std::cerr << "[ERROR] " << message << std::endl

#endif
