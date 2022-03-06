#include "application.h"
#include "define.h"
#include "log.h"
#include "config.h"
#include "writer.h"

#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <unistd.h>

namespace experience {


Application::Application() {

}

void Application::init_log() {
    // init default log
    SingletonPtr<experience::Logger>::get_instance()->init_default();
}


void Application::start() {
    EXPERIENCE_INFO(">>>>>> application start");
    // hardware module
    ModuleCfgCor::ptr hardware = ModuleCfgCor::ptr(new HardModule);
    
    QueueInterface::ptr que = QueueInterface::ptr(new Queue());

    sleep(5000);

    // create thread
    auto th = std::thread(std::bind(&ModuleCfgCor::collect, hardware, que));
    th.join();
}

void Application::init_config(std::vector<ConfigInterface::ptr> vec) {
    // load all config
    for (auto iter : vec)
        iter->load_from_file(iter->get_config_file());
}


}