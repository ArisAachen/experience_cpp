#include "config.h"
#include "log.h"
#include "utils.h"

#include <exception>
#include <stdexcept>
#include <sys/types.h>

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
        if (!info_->SerializeToString(&message))
            throw std::invalid_argument("message not valid");
        // try to write to file
        if (!FileUtils::save_to_file(message, filename)) 
            throw std::logic_error("write file failed");
        
        // write successfully
        EXPERIENCE_FMT_INFO("config write to %s successfully, info: %s", filename.c_str(), info_->DebugString().c_str());
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

        if (!info_->ParseFromString(message)) 
            throw std::invalid_argument("message not valid");

        EXPERIENCE_FMT_INFO("config load from %s successfully, info: %s", filename.c_str(), info_->DebugString().c_str());
    } catch(std::exception & err) {
        EXPERIENCE_FMT_ERR("load from file failed", err.what());
        return;
    } 
}

// check if need update
template<typename T>
bool  Module<T>::need_update() {

    return false;
}

template<typename T>
void  Module<T>::collect(QueueInterface::ptr que) {

}

template<typename T>
void  Module<T>::handler(ReqResult::ptr result) {
    
}

}