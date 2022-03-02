#ifndef __EXPERIENCE_SRC_UTILS_H__
#define __EXPERIENCE_SRC_UTILS_H__

#include "macro.h"
#include "define.h"

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

// StringUtils use to operate string
class StringUtils {
public:
    /**
     * @brief sprintf string
     * @param[in] fmt 
     * @param[in] va args 
     */
    static const std::string sprintf(const char* fmt, ...); 

    /**
     * @brief encode msg with base64
     * @param[in] msg origin message
     */
    static const std::string base64_encode(const std::string & msg);

    /**
     * @brief decode message
     * @param[in] msg origin message
     */
    static const std::string base64_decode(const std::string & msg);

    /**
     * @brief use pkcs7 to pad message
     * @param[in] message message 
     * @param[in] block_size padding size
     */
    static const std::string pkcs7_encode(const std::string & message, int block_size);

    /**
     * @brief use pkcs7 to unpad message
     * @param[in] message message
     */
    static const std::string pkcs7_decode(const std::string & message);

    /**
     * @brief use aes to encode msg
     * @param[in] msg msg
     */
    static const CryptResult::ptr aes_encode(const std::string & msg);

    /**
     * @brief use aes decode message
     * @param[in] crypt_msg crypted message
     */
    static const std::string aes_decode(CryptResult::ptr crypt_msg);

    /**
     * @brief use rsa to encode message
     * @param[in] key rsa public key
     * @param[in] msg message
     */
    static const std::string rsa_encode(const std::string & key, const std::string & msg);

    /**
     * @brief use rsa to decode message
     * @param[in] key rsa private key
     * @param[in] msg message
     */
    static const std::string rsa_decode(const std::string & key, const std::string & msg);

    /**
     * @brief join string
     * @tparam T must be container
     * @param[in] container container obj
     * @param[in] sep sep 
     */
    template<typename T>
    static const std::string join(T & container, const std::string & sep);
};

// FileUtils use to operate file
class FileUtils  {
public:
    /**
     * @brief save mesage to file
     * @param[in] msg msg
     * @param[in] filepath file path
     */
    static bool save_to_file(const std::string & msg, const std::string & filepath);

    /**
     * @brief load message from file
     * @param[in] msg msg
     * @param[in] filepath file path
     */
    static bool load_from_file(std::string & msg, const std::string & filepath);
};

class SystemInfo {
public:
    /**
     * @brief Get user host name
     */
    static const std::string user();

    /**
     * @brief Get pid
     */
    static uint64_t pid();

    /**
     * @brief Get process name
     */
    static const std::string process_name();
};


}


#endif