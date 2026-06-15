/*
** Utils.cpp
** ---------
** Implementations of the small helper utilities.
*/

#include "Utils.hpp"
#include <sys/stat.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace Utils {

// Trim leading and trailing ASCII whitespace
std::string trim(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start])))
        ++start;
    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1])))
        --end;
    return s.substr(start, end - start);
}

// Split by a single delimiter character; empty tokens are included
std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> result;
    std::istringstream ss(s);
    std::string token;
    while (std::getline(ss, token, delim))
        result.push_back(token);
    return result;
}

// Use stat() to check for a regular file
bool isFile(const std::string& path) {
    struct stat st;
    if (::stat(path.c_str(), &st) != 0)
        return false;
    return S_ISREG(st.st_mode);
}

// Use stat() to check for a directory
bool isDirectory(const std::string& path) {
    struct stat st;
    if (::stat(path.c_str(), &st) != 0)
        return false;
    return S_ISDIR(st.st_mode);
}

// Read entire file into a std::string using ifstream
bool readFile(const std::string& path, std::string& out) {
    std::ifstream file(path.c_str(), std::ios::binary);
    if (!file.is_open()) return false;
    // Read all bytes at once via stringstream buffer
    std::ostringstream ss;
    ss << file.rdbuf();
    out = ss.str();
    return true;
}

// Extract file extension including the dot
std::string fileExtension(const std::string& path) {
    size_t dot = path.rfind('.');
    if (dot == std::string::npos) return "";
    // Make sure the dot isn't part of a directory name
    size_t slash = path.rfind('/');
    if (slash != std::string::npos && dot < slash) return "";
    return path.substr(dot);
}

// Join path segments with exactly one '/' in between
std::string joinPath(const std::string& base, const std::string& rel) {
    if (base.empty()) return rel;
    if (rel.empty()) return base;
    bool baseSlash = (base[base.size() - 1] == '/');
    bool relSlash  = (rel[0] == '/');
    if (baseSlash && relSlash)
        return base + rel.substr(1);
    if (!baseSlash && !relSlash)
        return base + "/" + rel;
    return base + rel;
}

// Simple size_t to string without std::to_string (C++98)
std::string sizeToString(size_t n) {
    std::ostringstream oss;
    oss << n;
    return oss.str();
}

// Case-insensitive comparison
bool iequals(const std::string& a, const std::string& b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i) {
        if (std::tolower(static_cast<unsigned char>(a[i])) !=
            std::tolower(static_cast<unsigned char>(b[i])))
            return false;
    }
    return true;
}

std::string toLower(const std::string& s) {
    std::string result = s;
    for (size_t i = 0; i < result.size(); ++i)
        result[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(result[i])));
    return result;
}

} // namespace Utils
