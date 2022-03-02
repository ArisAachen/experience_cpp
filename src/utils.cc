#include "utils.h"
#include "define.h"
#include "macro.h"


#include <cstdint>
#include <fstream>
#include <ios>
#include <string>
#include <unistd.h>

#include <cryptopp/base64.h>
#include <cryptopp/filters.h>
#include <cryptopp/osrng.h>

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

// base64 encode message
const std::string StringUtils::base64_encode(const std::string & msg) {
    std::string result;
    CryptoPP::StringSource(msg, true, new CryptoPP::Base64Encoder(new CryptoPP::StringSink(result)));
    return result;
}

// base64 decode message
const std::string StringUtils::base64_decode(const std::string & msg) {
    std::string result;
    CryptoPP::StringSource(msg, true, new CryptoPP::Base64Decoder(new CryptoPP::StringSink(result)));
    return result;
}

// pkcs7 padding
const std::string StringUtils::pkcs7_encode(const std::string & message, int block_size) {
    // padding size must 
    EXPERIENCE_ASSERT(block_size > 0);
    std::string result = message;
    int pad_size = block_size - message.length() % block_size;
    result.append(std::to_string(pad_size), pad_size);
    return result;
}

// pkcs7 unpadding
const std::string StringUtils::pkcs7_decode(const std::string &message) {
    EXPERIENCE_ASSERT(message.length() > 0);
    std::string result = message;
    int pad = static_cast<int>(message.back());
    result.erase(result.end() - pad, result.end());
    return result;
}

// AES encode message
const CryptResult::ptr StringUtils::aes_encode(const std::string & msg) {
}

// AES decode message
const std::string StringUtils::aes_decode(CryptResult::ptr crypt_msg) {

}

const std::string StringUtils::rsa_encode(const std::string &key, const std::string &msg) {
    CryptoPP::AutoSeededRandomPool pool;
}

const std::string StringUtils::rsa_decode(const std::string &key, const std::string &msg) {

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