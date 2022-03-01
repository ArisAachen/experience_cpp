#ifndef __EXPERIENCE_SRC_UTILS_H__
#define __EXPERIENCE_SRC_UTILS_H__

#include "macro.h"

#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <sched.h>
#include <sstream>
#include <string>
#include <fstream>
#include <unistd.h>

namespace experience {

class StringUtils {
public:
    /**
     * @brief sprintf string
     * @param fmt 
     * @param ... 
     */
    static const std::string sprintf(const char* fmt, ...) {
        va_list ap;
        
        va_start(ap, fmt);
        char* buf = nullptr;
        size_t size = vasprintf(&buf, fmt, ap);
        EXPERIENCE_ASSERT(size < 0);
        va_end(ap);
        return "";
    }





};

class SystemInfo {
public:
    /**
     * @brief Get user host name
     */
    static const std::string user() {
        static std::string user;
        if (user == "") {
            std::ifstream file("/etc/hostname");
            // check if open file successfully
            if (file.is_open()) {
                return "";
            }
            file >> user;
            // close file
            file.close();
        }
        return user;
    }

    /**
     * @brief Get pid
     */
    static uint64_t pid() {
        return getpid();
    }

    static const std::string process_name() {
        return "";
    }
};



}


#endif