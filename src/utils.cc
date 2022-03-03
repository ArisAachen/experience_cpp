#include "utils.h"
#include "define.h"
#include "macro.h"

#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <ios>
#include <memory>
#include <string>
#include <algorithm>
#include <random>

#include <unistd.h>
#include <cryptopp/base64.h>
#include <cryptopp/rsa.h>
#include <cryptopp/aes.h>
#include <cryptopp/config_int.h>
#include <cryptopp/filters.h>
#include <cryptopp/integer.h>
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

const std::string StringUtils::random(int size) {
    // TODO should optimize code
    static const std::string seeds = std::string("0123456789_") + std::string("ABCDEFGHIJKLMNOPQRSTUVWXYZ") 
        + std::string("abcdefghijklmnopqrstuvwxyz");
    // random
    std::default_random_engine random(time(nullptr));
    std:std::uniform_int_distribution<unsigned> range(0, sizeof(seeds) - 1);
    std::string result;
    for (int index = 0; index < size; index++) {
        result += seeds[range(random)];
    }
    return result;
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
    auto iter = container.cbegin()++;
    for (iter ; iter != container.cend(); iter++) {
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
    std::string seed = random(32);
    CryptoPP::byte key[CryptoPP::AES::DEFAULT_KEYLENGTH];
    return nullptr;
}

// AES decode message
const std::string StringUtils::aes_decode(CryptResult::ptr crypt_msg) {

}

// rsa encode
const std::string StringUtils::rsa_encode(const std::string &key, const std::string &msg) {
    // string source
    CryptoPP::StringSource source(key, true);
    CryptoPP::RSA::PublicKey rsa_key;
    rsa_key.Load(source);
    // encrypt
    std::string result;
    CryptoPP::AutoSeededRandomPool pool;
    CryptoPP::RSAES_OAEP_SHA_Encryptor e(rsa_key);
    CryptoPP::StringSource(msg, true, new CryptoPP::PK_EncryptorFilter(pool, e, new CryptoPP::StringSink(result)));
    return result;
}

// rsa decode
const std::string StringUtils::rsa_decode(const std::string &key, const std::string &msg) {
    // string source
    CryptoPP::StringSource source(key, true);
    CryptoPP::RSA::PrivateKey rsa_key;
    rsa_key.Load(source);
    // decrypt
    std::string result;
    CryptoPP::AutoSeededRandomPool pool;
    CryptoPP::RSAES_OAEP_SHA_Decryptor e(rsa_key);
    CryptoPP::StringSource(msg, true, new CryptoPP::PK_DecryptorFilter(pool, e, new CryptoPP::StringSink(result)));
    return result;
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

const std::string SystemInfo::get_zone() {
    static std::string zone;
    if (zone == "") {
        if (!FileUtils::load_from_file(zone, zone_file)) {
            return "";
        }
    }
    return zone;
}

static std::string unid = "";

// get pid 
uint64_t SystemInfo::pid() {
    return getpid();
}

// get unid
const std::string SystemInfo::get_unid() {
    return unid;
}

void SystemInfo::set_unid(const std::string &id) {
    unid = id;
}

// post aid
const std::string SystemInfo::get_aid() {
    return post_aid;
}

const std::string SystemInfo::get_content_type() {
    return post_content_type;
}

}