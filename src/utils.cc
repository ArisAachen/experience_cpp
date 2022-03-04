#include "utils.h"
#include "dbus.h"
#include "define.h"
#include "macro.h"

#include <cctype>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <ios>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <algorithm>
#include <random>
#include <vector>
#include <map>

#include <cryptopp/base64.h>
#include <cryptopp/rsa.h>
#include <cryptopp/aes.h>
#include <cryptopp/config_int.h>
#include <cryptopp/filters.h>
#include <cryptopp/integer.h>
#include <cryptopp/osrng.h>
#include <core/dbus/bus.h>
#include <core/dbus/service.h>
#include <core/dbus/traits/service.h>
#include <core/dbus/types/object_path.h>
#include <core/dbus/asio/executor.h>
#include <boost/format/format_fwd.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/predicate.hpp>
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
const std::string CryptorUtils::base64_encode(const std::string & msg) {
    std::string result;
    CryptoPP::StringSource(msg, true, new CryptoPP::Base64Encoder(new CryptoPP::StringSink(result)));
    return result;
}

// base64 decode message
const std::string CryptorUtils::base64_decode(const std::string & msg) {
    std::string result;
    CryptoPP::StringSource(msg, true, new CryptoPP::Base64Decoder(new CryptoPP::StringSink(result)));
    return result;
}

// pkcs7 padding
const std::string CryptorUtils::pkcs7_encode(const std::string & message, int block_size) {
    // padding size must 
    EXPERIENCE_ASSERT(block_size > 0);
    std::string result = message;
    int pad_size = block_size - message.length() % block_size;
    result.append(std::to_string(pad_size), pad_size);
    return result;
}

// pkcs7 unpadding
const std::string CryptorUtils::pkcs7_decode(const std::string &message) {
    EXPERIENCE_ASSERT(message.length() > 0);
    std::string result = message;
    int pad = static_cast<int>(message.back());
    result.erase(result.end() - pad, result.end());
    return result;
}

// AES encode message
const CryptResult::ptr CryptorUtils::aes_encode(const std::string & msg) {
    std::string seed = StringUtils::random(32);
    CryptoPP::byte key[CryptoPP::AES::DEFAULT_KEYLENGTH];
    return nullptr;
}

// AES decode message
const std::string CryptorUtils::aes_decode(CryptResult::ptr crypt_msg) {

}

// rsa encode
const std::string CryptorUtils::rsa_encode(const std::string &key, const std::string &msg) {
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
const std::string CryptorUtils::rsa_decode(const std::string &key, const std::string &msg) {
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

const std::string SystemInfo::get_machine_id() {
    static std::string machine_id;
    if (machine_id == "") {
        if (!FileUtils::load_from_file(machine_id, machine_id_file)) {
            return "";
        }
    }
    return machine_id;
}

const std::string SystemInfo::get_apt_token() {
    static std::string apt_token;
    if (apt_token == "") {
        if (!FileUtils::load_from_file(apt_token, apt_token_file)) {
            return "";
        }
        std::vector<std::string> vec;
        boost::algorithm::split(vec, apt_token, " ");

    }
    return apt_token;
}

const std::string SystemInfo::get_active_code() {
    return "";
}

class SysModule {
public:
    typedef std::shared_ptr<SysModule> ptr;
    /**
     * @brief parse info 
     * @param[in] info multi info
     */
    virtual void parse(const std::string & info) = 0;
    /**
     * @brief Get the model object
     */
    virtual const std::string get_model() = 0;

    /**
     * @brief Get the id object
     */
    virtual const std::string get_id() = 0;

    /**
     * @brief Get the module dmidecode object
     */
    virtual std::string get_module_dmidecode() = 0;
};

class CpuModule : public SysModule {
public:
    /**
     * @brief parse cpu info
     * @param[in] info info string
     */
    virtual void parse(const std::string & info) override {
        // trim all space
        boost::algorithm::trim_left_if(info, " ");
        std::vector<std::string> vec;
        // split
        boost::algorithm::split(vec, info, 2);
        // check size
        if (vec.size() < 2) {
            return;
        }
        // TODO should optimize code
        if (vec.front() == version_index) {
            version_ = vec.at(1);
        } else if (vec.front() == core_count_inex) {
            core_ = vec.at(1);
        } else if (vec.front() == thread_count_index) {
            thread_ = vec.at(1);
        } else if (vec.front() == id_index) {
            id_ = vec.at(1);
        }
    }
    /**
     * @brief Get the model object
     */
    virtual const std::string get_model()  override {
        auto fm = boost::format("%1%(%2% core/%3% thread)") % version_ % core_ % thread_;
        return fm.str();
    }

    /**
     * @brief Get the id object
     */
    virtual const std::string get_id() override {
        return id_;
    } 

    /**
     * @brief Get the module dmidecode object
     */
    virtual std::string get_module_dmidecode() override {
        return "dmidecode_4";
    }

private:
    /// version index
    const std::string version_index = "Version";
    /// core count index
    const std::string core_count_inex = "Core Count";
    /// thread count index
    const std::string thread_count_index = "Thread Count";
    /// id index
    const std::string id_index = "ID";

private:
    /// cpu version
    std::string version_ {""};
    /// cpu core count
    std::string core_ {""};
    /// cpu thread
    std::string thread_ {""};
    /// cpu id
    std::string id_ {""};
};

class BaseboardModule : public SysModule {
public:
    /**
     * @brief parse cpu info
     * @param[in] info info string
     */
    virtual void parse(const std::string & info) override {
        // trim all space
        boost::algorithm::trim_left_if(info, " ");
        std::vector<std::string> vec;
        // split
        boost::algorithm::split(vec, info, 2);
        // check size
        if (vec.size() < 2) {
            return;
        }
        // TODO should optimize code
        if (vec.front() == manual_index) {
            manual_ = vec.at(1);
        } else if (vec.front() == version_inex) {
            version_ = vec.at(1);
        } else if (vec.front() == serial_index) {
            serial_ = vec.at(1);
        }
    }
    /**
     * @brief Get the model object
     */
    virtual const std::string get_model()  override {
        return manual_ + " " + version_;
    }

    /**
     * @brief Get the id object
     */
    virtual const std::string get_id() override {
        return serial_;
    } 

    /**
     * @brief Get the module dmidecode object
     */
    virtual std::string get_module_dmidecode() override {
        return "dmidecode_1";
    }    

private:
    /// version index
    const std::string manual_index = "Manufacturer";
    /// core count index
    const std::string version_inex = "Version";
    /// thread count index
    const std::string serial_index = "Serial Number";

private:
    /// board manufacturer
    std::string manual_ {""};
    /// board version
    std::string version_ {""};
    /// board serial number
    std::string serial_ {""};
};

class MemoryModule : public SysModule {
public:
    /**
     * @brief parse cpu info
     * @param[in] info info string
     */
    virtual void parse(const std::string & info) override {
        // trim all space
        boost::algorithm::trim_left_if(info, " ");
        std::vector<std::string> vec;
        // split
        boost::algorithm::split(vec, info, 2);
        // check size
        if (vec.size() < 2) {
            return;
        }
        // TODO should optimize code
        if (vec.front() == manual_index) {
            manual_ = vec.at(1);
        } else if (vec.front() == part_inex) {
            part_ = vec.at(1);
        } else if (vec.front() == size_index) {
            size_ = vec.at(1);
        } else if (vec.front() == serial_index) {
            serial_ = vec.at(1);
        }
    }
    /**
     * @brief Get the model object
     */
    virtual const std::string get_model()  override {
        auto fm = boost::format("%1% %2%(%3%)") % manual_ % part_ % size_;
        return fm.str();
    }

    /**
     * @brief Get the id object
     */
    virtual const std::string get_id() override {
        return serial_;
    } 

    /**
     * @brief Get the module dmidecode object
     */
    virtual std::string get_module_dmidecode() override {
        return "dmidecode_17";
    }   

private:
    /// disk model family
    const std::string manual_index = "Manufacturer";
    /// disk device model
    const std::string part_inex = "Device Model";
    /// disk user capacity
    const std::string size_index = "Size";
    /// disk serial
    const std::string serial_index = "Serial Number";

private:
    /// memory manufacturer
    std::string manual_ {""};
    /// memory part number
    std::string part_ {""};
    /// memory size
    std::string size_ {""};
    /// memory serial number
    std::string serial_ {""};
};

class DiskModule : public SysModule {
public:
    /**
     * @brief parse disk info
     * @param[in] info info string
     */
    virtual void parse(const std::string & info) override {
        // trim all space
        boost::algorithm::trim_left_if(info, " ");
        std::vector<std::string> vec;
        // split
        boost::algorithm::split(vec, info, 2);
        // check size
        if (vec.size() < 2) {
            return;
        }
        // TODO should optimize code
        if (vec.front() == family_index) {
            family_ = vec.at(1);
        } else if (vec.front() == model_inex) {
            model_ = vec.at(1);
        } else if (vec.front() == user_cap_index || vec.front() == nvm_cap_index) {
            cap_ = vec.at(1);
        } else if (vec.front() == serial_index) {
            serial_ = vec.at(1);
        }
    }
    /**
     * @brief Get the model object
     */
    virtual const std::string get_model()  override {
        if (family_ == "") {
            auto fm = boost::format("%s(%s)") % model_ % cap_;
            return fm.str();
        } 
        auto fm = boost::format("%1 %2(%3)") % family_ % model_ % cap_;
        return fm.str();
    }

    /**
     * @brief Get the id object
     */
    virtual const std::string get_id() override {
        return serial_;
    } 

    /**
     * @brief Get the module dmidecode object
     */
    virtual std::string get_module_dmidecode() override {
        return "lsblk_d";
    }   

private:
    /// disk model family
    const std::string family_index = "Model Family";
    /// disk device model
    const std::string model_inex = "Device Model";
    /// disk user capacity
    const std::string user_cap_index = "User Capacity";
    /// disknvm capacity
    const std::string nvm_cap_index = "Total NVM Capacity";
    /// disk serial
    const std::string serial_index = "Serial Number";

private:
    /// disk family
    std::string family_ {""};
    /// disk model
    std::string model_ {""};
    /// disk cap
    std::string cap_ {""};
    /// disk serial number
    std::string serial_ {""};
};

class GpuModule : public SysModule {
public:
    /**
     * @brief parse cpu info
     * @param[in] info info string
     */
    virtual void parse(const std::string & info) override {
        // trim all space
        boost::algorithm::trim_left_if(info, " ");
        std::vector<std::string> vec;
        // split
        boost::algorithm::split(vec, info, 2);
        // check size
        if (vec.size() < 2) {
            return;
        }
        // TODO should optimize code
        if (vec.front() == model_index) {
            model_ = vec.at(1);
        } else if (vec.front() == id_inex) {
            serial_ = vec.at(1);
        }
    }
    /**
     * @brief Get the model object
     */
    virtual const std::string get_model()  override {
        return model_;
    }

    /**
     * @brief Get the id object
     */
    virtual const std::string get_id() override {
        return serial_;
    } 

    /**
     * @brief Get the module dmidecode object
     */
    virtual std::string get_module_dmidecode() override {
        return "gpu";
    }   

private:
    /// disk model family
    const std::string model_index = "Model";
    /// disk device model
    const std::string id_inex = "Serial ID";

private:
    /// memory manufacturer
    std::string model_ {""};
    /// memory serial number
    std::string serial_ {""};
};

class NetcardModule : public SysModule {
public:
    /**
     * @brief parse netcard info
     * @param[in] info info string
     */
    virtual void parse(const std::string & info) override {
        // trim all space
        boost::algorithm::trim_left_if(info, " ");
        std::vector<std::string> vec;
        // split
        boost::algorithm::split(vec, info, 2);
        // check size
        if (vec.size() < 2) {
            return;
        }
        // TODO should optimize code
        if (vec.front() == model_index) {
            model_ = vec.at(1);
        } else if (vec.front() == perm_index) {
            perm_ = vec.at(1);
        }
    }

    /**
     * @brief Get the model object
     */
    virtual const std::string get_model()  override {
        return model_;
    }

    /**
     * @brief Get the id object
     */
    virtual const std::string get_id() override {
        return "";
    } 

    /**
     * @brief Get the module dmidecode object
     */
    virtual std::string get_module_dmidecode() override {
        return "net";
    }   

private:
    /// model
    const std::string model_index = "Model";
    /// perm
    const std::string perm_index = "Permanent HW Address";

private:
    /// netcard model
    std::string model_ {""};
    /// perm hardware
    std::string perm_ {""};
};

class SysModuleFactory {
public:
    static SysModule::ptr create_sys_module(SysModuleIndex index) {
        // TODO code optimize
        switch (index) {
        case SysModuleIndex::CpuModuleIndex:
            return SysModule::ptr(new CpuModule);
        case SysModuleIndex::BoardModuleIndex:
            return SysModule::ptr(new BaseboardModule);
        case SysModuleIndex::MemoryModuleIndex:
            return SysModule::ptr(new MemoryModule);            
        case SysModuleIndex::DiskModuleValue:
            return SysModule::ptr(new DiskModule);     
        case SysModuleIndex::GpuModuleIndex:
            return SysModule::ptr(new GpuModule);     
        case SysModuleIndex::NetModuleIndex:
            return SysModule::ptr(new NetcardModule);
        default:
            return nullptr;
        }
    }
};

typedef core::dbus::Bus::Ptr Bus;
Bus the_system_bus() {
    // get system bus
    static Bus system_bus = std::make_shared<core::dbus::Bus>(core::dbus::WellKnownBus::system);
    return system_bus;
}

// generate get hardware message
std::vector<HardwareMsg::ptr> DeviceUtils::generate(SysModuleIndex module_index) {
    // system bus
    auto sys_bus = the_system_bus();
    sys_bus->install_executor(core::dbus::asio::make_executor(sys_bus));
    // dbus obj
    auto device_manager = core::dbus::Service::use_service(sys_bus, core::dbus::traits::Service<core::DeviceManager>::interface_name());
    auto manager_obj = device_manager->object_for_path(core::dbus::types::ObjectPath(device_manager_dbus_path));
    // create module
    auto module = SysModuleFactory::create_sys_module(module_index);
    // device info
    std::string info = manager_obj->invoke_method_synchronously<core::DeviceManager::getInfo, std::string, std::string>(module->get_module_dmidecode());

    // split to part
    std::vector<std::string> parts;
    boost::algorithm::split(parts, info, "\n");

    // read part line
    std::stringbuf ss;
    std::vector<HardwareMsg> hw_vec;
    for (auto iter : parts) {
        // reset string
        ss.str("");
        ss << iter;
        std::string part;
        while (std::getline(ss, part)) {
            // parse
            module->parse(part);
        }
        HardwareMsg::ptr info;
        info->model = module->get_model();
        info->id = module->get_id();
        hw_vec.emplace_back(info);
    }
    return hw_vec;
}

std::vector<HardwareMsg::ptr> DeviceUtils::get_cpu_info() {
    return generate(SysModuleIndex::CpuModuleIndex);
}

std::vector<HardwareMsg::ptr> DeviceUtils::get_board_info() {
    return generate(SysModuleIndex::BoardModuleIndex);
}

std::vector<HardwareMsg::ptr> DeviceUtils::get_memory_info() {
    return generate(SysModuleIndex::MemoryModuleIndex);
}

std::vector<HardwareMsg::ptr> DeviceUtils::get_disk_info() {
    return generate(SysModuleIndex::DiskModuleValue);
}

std::vector<HardwareMsg::ptr> DeviceUtils::get_smartdisk_info() {
    return generate(SysModuleIndex::SmartDiskModuleIndex);
}

std::vector<HardwareMsg::ptr> DeviceUtils::get_gpu_info() {
    return generate(SysModuleIndex::GpuModuleIndex);
}

std::vector<HardwareMsg::ptr> DeviceUtils::get_netcard_info() {
    return generate(SysModuleIndex::NetModuleIndex);
}



}