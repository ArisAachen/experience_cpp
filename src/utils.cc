#include "utils.h"
#include "dbus.h"
#include "define.h"
#include "macro.h"


#include <cctype>
#include <chrono>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <exception>
#include <fcntl.h>
#include <fstream>
#include <future>
#include <ios>
#include <iostream>
#include <math.h>
#include <memory>
#include <sstream>
#include <algorithm>
#include <random>
#include <string_view>
#include <filesystem>
#include <sys/stat.h>
#include <system_error>
#include <thread>
#include <vector>
#include <map>

#include <unistd.h>
#include <fcntl.h>
#include <cryptopp/config_int.h>
#include <cryptopp/filters.h>
#include <cryptopp/integer.h>
#include <cryptopp/osrng.h>
#include <cryptopp/base64.h>
#include <cryptopp/rsa.h>
#include <cryptopp/aes.h>
#include <core/dbus/bus.h>
#include <core/dbus/service.h>
#include <core/dbus/result.h>
#include <core/dbus/traits/service.h>
#include <core/dbus/types/object_path.h>
#include <core/dbus/asio/executor.h>
#include <core/dbus/object.h>
#include <boost/format/format_fwd.hpp>
#include <boost/format.hpp>
#include <boost/range/concepts.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/constants.hpp>
#include <boost/process/detail/child_decl.hpp>
#include <boost/process/io.hpp>
#include <boost/process/pipe.hpp>
#include <boost/process.hpp>



namespace experience {

const std::string StringUtils::sprintf(const char* fmt, ...) {
        va_list ap;
        
        va_start(ap, fmt);
        char* buf = nullptr;
        size_t size = vasprintf(&buf, fmt, ap);
        EXPERIENCE_ASSERT(size < 0);
        va_end(ap);
        std::string result(buf);
        free(buf);
        return result;    
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

// execute command
const std::string StringUtils::execute_cmd(const std::string &cmd) {
    std::string msg;
    // execute and get result
    boost::process::child proc(cmd, boost::process::std_out > msg);
    // wait proc
    proc.wait();
    // get result 
    return msg;
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
    return "";
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

bool FileUtils::create_dir(const std::string &path) {
    // create dir
    std::error_code code;
    std::filesystem::create_directory(path, code);
    // check if create dir success
    if (code) 
        return false;
    return true;
}

bool FileUtils::create_file(const std::string &path) {
    int fd = open(path.c_str(), O_RDWR | O_CREAT , S_IRWXU | S_IRWXG | S_IRWXO);
    if (fd == -1) {
        return false;
    }
    close(fd);
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
        boost::algorithm::split(vec, apt_token, boost::is_any_of(","), boost::token_compress_on);
    }
    return apt_token;
}

const std::string SystemInfo::get_active_code() {
    return "";
}

static bool experience_enable = false;

const bool SystemInfo::get_experience_enable() {
    return experience_enable;
}

void SystemInfo::set_experience_enable(bool enable) {
    experience_enable = enable;
}

class SysModule {
public:
    typedef std::shared_ptr<SysModule> ptr;
    /**
     * @brief parse info 
     * @param[in] info multi info
     */
    virtual void parse(std::string & info) = 0;
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
    virtual void parse(std::string & info) override {
        // trim all space
        boost::algorithm::trim_left_if(info, boost::algorithm::is_space());
        std::vector<std::string> vec;
        // split
        boost::algorithm::split(vec, info, boost::is_any_of(";"), boost::token_compress_on);
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
    virtual void parse(std::string & info) override {
        // trim all space
        boost::algorithm::trim_left_if(info, boost::algorithm::is_space());
        std::vector<std::string> vec;
        // split
        boost::algorithm::split(vec, info, boost::is_any_of(":"), boost::token_compress_on);
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
    virtual void parse(std::string & info) override {
        // trim all space
        boost::algorithm::trim_left_if(info, boost::algorithm::is_space());
        std::vector<std::string> vec;
        // split
        boost::algorithm::split(vec, info, boost::is_any_of(";"), boost::token_compress_on);
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
    virtual void parse(std::string & info) override {
        // trim all space
        boost::algorithm::trim_left_if(info, boost::algorithm::is_space());
        std::vector<std::string> vec;
        // split
        boost::algorithm::split(vec, info, boost::is_any_of(";"), boost::token_compress_on);
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
    virtual void parse(std::string & info) override {
        // trim all space
        boost::algorithm::trim_left_if(info, boost::algorithm::is_space());
        std::vector<std::string> vec;
        // split
        boost::algorithm::split(vec, info, boost::is_any_of(";"), boost::token_compress_on);
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
    virtual void parse(std::string & info) override {
        // trim all space
        boost::algorithm::trim_left_if(info, boost::algorithm::is_space());
        std::vector<std::string> vec;
        // split
        boost::algorithm::split(vec, info, boost::is_any_of(";"), boost::token_compress_on);
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

Bus the_session_bus() {
    // get system bus
    static Bus sesson_bus = std::make_shared<core::dbus::Bus>(core::dbus::WellKnownBus::session);
    return sesson_bus;
}

// generate get hardware message
std::vector<HardwareMsg::ptr> DeviceUtils::generate(SysModuleIndex module_index) {
    // should wait util service is on   
    DBusUtils::wait_system_dbus(core::dbus::traits::Service<core::DeviceManager>::interface_name());
    // info 
    core::dbus::Result<std::string> info;
    SysModule::ptr hardware_module;
    try {
        // system bus
        auto sys_bus = the_system_bus();
        sys_bus->install_executor(core::dbus::asio::make_executor(sys_bus));
        // dbus obj
        auto device_manager = core::dbus::Service::use_service(sys_bus, core::dbus::traits::Service<core::DeviceManager>::interface_name());
        auto manager_obj = device_manager->object_for_path(core::dbus::types::ObjectPath(device_manager_dbus_path));
        // create module
        hardware_module = SysModuleFactory::create_sys_module(module_index);
        // device info
        info = manager_obj->invoke_method_synchronously<core::DeviceManager::getInfo, std::string, std::string>(hardware_module->get_module_dmidecode());        
    } catch (std::exception & e) {
        return std::vector<HardwareMsg::ptr>{};
    }

    // split to part
    std::vector<std::string> parts;
    boost::algorithm::split(parts, info.value(), boost::is_any_of("\n"), boost::token_compress_on);

    // read part line
    std::istringstream ss;
    std::vector<HardwareMsg::ptr> hw_vec;
    for (auto iter : parts) {
        // reset string
        ss.clear();
        ss.str(iter);
        std::string part;
        while (std::getline(ss, part)) {
            // parse
            hardware_module->parse(part);
        }
        HardwareMsg::ptr info;
        info->model = hardware_module->get_model();
        info->id = hardware_module->get_id();
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

bool DBusUtils::check_session_dbus_exist(const std::string &name) {
    try {
        // create session bus 
        auto session_bus = the_session_bus();
        // install
        session_bus->install_executor(core::dbus::asio::make_executor(session_bus));
        auto bus = core::dbus::Service::use_service(session_bus, core::dbus::traits::Service<core::DBus>::interface_name());
        auto bus_obj = bus->add_object_for_path(core::dbus::types::ObjectPath("/org/freedesktop/DBus"));
        core::dbus::Result<bool> exist = bus_obj->invoke_method_synchronously<core::DBus::NameHasOwner, bool, std::string>(name);
        return exist.value();
    } catch (std::exception & e) {
        std::cout << e.what() << std::endl;
        return false;
    }
}

bool DBusUtils::check_system_dbus_exist(const std::string &name) {
    try {
        // create session bus 
        auto system_bus = the_system_bus();
        // install
        system_bus->install_executor(core::dbus::asio::make_executor(system_bus));
        auto bus = core::dbus::Service::use_service(system_bus, core::dbus::traits::Service<core::DBus>::interface_name());
        auto bus_obj = bus->add_object_for_path(core::dbus::types::ObjectPath("/org/freedesktop/DBus"));
        core::dbus::Result<bool> exist = bus_obj->invoke_method_synchronously<core::DBus::NameHasOwner, bool, std::string>(name);
        return exist.value();
    } catch (std::exception & e) {
        std::cout << "system bus " << e.what();
        return false;
    }
}

void DBusUtils::wait_session_dbus(const std::string &name) {
    while (true) {
        // wait util bus found
        if (check_session_dbus_exist(name))
            break;
        // sleep 5 seconds and retry
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

void DBusUtils::wait_system_dbus(const std::string &name) {
    while (true) {
        // wait util bus found
        if (check_system_dbus_exist(name))
            break;
        // sleep 5 seconds and retry
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

}