#include "utils.h"

#include <cstdint>
#include <fstream>
#include <ios>
#include <string>
#include <unistd.h>

namespace experience {

const std::string StringUtils::sprintf(const char* fmt, ...) {
        va_list ap;
        
        va_start(ap, fmt);
        char* buf = nullptr;
        size_t size = vasprintf(&buf, fmt, ap);
        EXPERIENCE_ASSERT(size < 0);
        va_end(ap);
        return "";    
}

template<typename T>
const std::string StringUtils::join(T & container, const std::string &sep) {
    // check size
    switch (container.size()) {
    case 0:
        return "";
    case 1:
        return container[0];
    }

    // get first 
    std::string result = container[0];
    for (auto iter : container) {
        result.append(sep);
        result.append(*iter);
    }
}

// save to file
bool FileUtils::save_to_file(const std::string &msg, const std::string &filepath) {
    std::ofstream file(filepath, std::ios_base::out | std::ios_base::binary);
    // check if open successfully
    if (!file.is_open())
        return false;
    file << msg;
    file.close();
    return true;
}

// load from file
bool FileUtils::load_from_file(std::string &msg, const std::string &filepath) {
    std::ifstream file(filepath, std::ios_base::in | std::ios_base::binary);
    // check if open successfully
    if (!file.is_open())
        return false;
    file >> msg;
    file.close();
    return true;
}

// get user host name
const std::string SystemInfo::user() {
        static std::string user;
        if (user == "") {
            std::ifstream file("/etc/hostname");
            // check if open file successfully
            if (!file.is_open()) {
                return "";
            }
            file >> user;
            // close file
            file.close();
        }
        return user;    
}

// get pid 
uint64_t SystemInfo::pid() {
    return getpid();
}





}