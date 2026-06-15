#ifndef UTILS_HPP
#define UTILS_HPP

/*
** Utils.hpp
** ---------
** Small helper functions used across the codebase.
*/

#include <string>
#include <vector>
#include <sstream>

namespace Utils {

    // Trim leading and trailing whitespace from a string
    std::string trim(const std::string& s);

    // Split a string by a delimiter character
    std::vector<std::string> split(const std::string& s, char delim);

    // Convert an integer to string (C++98: no std::to_string)
    template<typename T>
    std::string toString(T val) {
        std::ostringstream oss;
        oss << val;
        return oss.str();
    }

    // Check if a filesystem path is a regular file
    bool isFile(const std::string& path);

    // Check if a filesystem path is a directory
    bool isDirectory(const std::string& path);

    // Read entire file into a string. Returns false on failure.
    bool readFile(const std::string& path, std::string& out);

    // Get file extension including dot, e.g. ".html" from "index.html"
    std::string fileExtension(const std::string& path);

    // Join two path segments, ensuring exactly one '/' between them
    std::string joinPath(const std::string& base, const std::string& rel);

    // Convert size_t to string
    std::string sizeToString(size_t n);

    // Case-insensitive string comparison
    bool iequals(const std::string& a, const std::string& b);

    // Convert string to lowercase
    std::string toLower(const std::string& s);

} // namespace Utils

#endif // UTILS_HPP
