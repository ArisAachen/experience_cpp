#include "config.h"
#include "define.h"
#include "config.h"
#include "define.pb.h"
#include "log.h"
#include "utils.h"

#include <functional>
#include <string>
#include <algorithm>
#include <exception>
#include <stdexcept>


#include <sys/types.h>
#include <google/protobuf/util/message_differencer.h>

namespace experience {

template<typename T>
Module<T>::Module() {
    
}

template<typename T>
Module<T>::~Module() {
    
}

// save hardware info to file
template<typename T>
void Module<T>::save_to_file(const std::string & filename) {
    try {
        std::string message;
        // try to serialize protobug
        if (!info_.SerializeToString(&message))
            throw std::invalid_argument("message not valid");
        // try to write to file
        if (!FileUtils::save_to_file(message, filename)) 
            throw std::logic_error("write file failed");
        
        // write successfully
        EXPERIENCE_FMT_INFO("config write to %s successfully, info: %s", filename.c_str(), info_.DebugString().c_str());
    } catch(std::exception & err) {
        EXPERIENCE_FMT_ERR("save to file failed", err.what());
        return;
    }
}

// load message from file
template<typename T>
void Module<T>::load_from_file(const std::string & filename) {
    try {
        std::string message;
        // load file
        if(!FileUtils::load_from_file(message, filename))
            throw std::logic_error("load file failed");

        if (!info_.ParseFromString(message)) 
            throw std::invalid_argument("message not valid");

        EXPERIENCE_FMT_INFO("config load from %s successfully, info: %s", filename.c_str(), info_.DebugString().c_str());
    } catch(std::exception & err) {
        EXPERIENCE_FMT_ERR("load from file failed, err: %s", err.what());
        return;
    } 
}

// check if need update
template<typename T>
bool  Module<T>::need_update() {

    return false;
}

// check if need update
template<typename T>
const std::string Module<T>::get_config_file() {
    return "";
}

template<typename T>
void  Module<T>::collect(QueueInterface::ptr que) {

}

template<typename T>
void  Module<T>::handler(ReqResult::ptr result) {
    
}

void HardModule::load_from_file(const std::string & filename) {
    // load from file
    Module<define::HardwareInfo>::load_from_file(filename);
    // set global uni
    SystemInfo::set_unid(info_.uni_id());
}

bool HardModule::need_update() {
    define::HardwareInfo hw_info;

#define ADD(hw) \
    auto vec_##hw = DeviceUtils::get_##hw##_info(); \
    for (auto iter : vec_##hw) { \
        auto mem = hw_info.add_##hw(); \
        mem->set_model(iter->model); \
        mem->set_id(iter->id); \
        EXPERIENCE_FMT_DEBUG("%s hardware info model:%s, id: %s", #hw, \
            iter->model.c_str(), iter->id.c_str()); \
    }

    // add hardware info
    ADD(cpu);
    ADD(board);
    ADD(memory);
    ADD(disk);
    ADD(gpu);
    ADD(netcard);

    // system info
    auto other = hw_info.other();
    other.set_machine(SystemInfo::get_machine_id());
    other.set_apt(SystemInfo::get_apt_token());
    other.set_active(SystemInfo::get_active_code());

    // get unid
    hw_info.set_uni_id(SystemInfo::get_unid());

    // check if current uni is not exist
    if (SystemInfo::get_unid() == "") {
        EXPERIENCE_INFO("uni is not exist, dont need to post req");
        info_ = hw_info;
        return true;
    }

    // check if is the same
    using util = google::protobuf::util::MessageDifferencer;
    if (util::Equals(hw_info, info_)) {
        EXPERIENCE_INFO("hardware is equal, dont need to post req");
        return false;
    }

    EXPERIENCE_INFO("hardware is diff, need to post req");
    // reset
    info_ = hw_info;
    return false;
}

const std::string HardModule::get_config_file() {
    return hardware_file;
}

// collect hardware info
void HardModule::collect(QueueInterface::ptr que) {
    // check if need update
    if (!need_update()) {
        return;
    }
    std::string hw_info;
    info_.SerializeToString(&hw_info);

    // post message
    ReqMessage::ptr req;
    req->tid = TidTyp::SystemInfoTid;
    req->call_back = std::bind(&HardModule::handler, this, std::placeholders::_1);
    req->vec.emplace_back(hw_info);
    que->push(req);
}

// save file
void HardModule::handler(ReqResult::ptr result) {
    // if write not success, should ignore
    if (result->code != ReqResultCode::WriteResultSuccess)  
        return;
    define::RcvUni uni;
    // parsel uni
    uni.ParseFromString(result->msg);
    info_.set_uni_id(uni.unid());
    // set global uni
    SystemInfo::set_unid(uni.unid());
    // save to file
    save_to_file(hardware_file);
}

}