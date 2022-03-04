#include "application.h"
#include "define.h"
#include "log.h"
#include "config.h"
#include "writer.h"

#include <functional>
#include <memory>
#include <string>
#include <thread>

namespace experience {

void Application::init_log() {
    // init default log
    SingletonPtr<experience::Logger>::get_instance()->init_default();
}


void Application::start() {
    // hardware module
    ModuleCfgCor::ptr hardware = ModuleCfgCor::ptr(new HardModule);
    
    QueueInterface::ptr que = QueueInterface::ptr(new Queue());

    // create thread
    std::thread(std::bind(&ModuleCfgCor::collect, hardware, que));
}

void Application::init_config(std::vector<ConfigInterface::ptr> vec) {
    // load all config
    for (auto iter : vec)
        iter->load_from_file(iter->get_config_file());
}


}