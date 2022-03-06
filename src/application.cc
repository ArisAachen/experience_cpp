#include "application.h"
#include "define.h"
#include "log.h"
#include "config.h"
#include "utils.h"
#include "writer.h"

#include <exception>
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <unistd.h>

namespace experience {


Application::Application() {
}

Application::~Application() {
    for (auto iter : threads_) 
        iter->join();
}

void Application::init_log() {
    // init default log
    SingletonPtr<experience::Logger>::get_instance()->init_default();
}


void Application::start() {
    EXPERIENCE_INFO(">>>>>> application start");
    // hardware module
    ModuleCfgCor::ptr hardware = ModuleCfgCor::ptr(new HardModule);
    ModuleWtrCor::ptr database = ModuleWtrCor::ptr (new DBModule);
    WriterInterface::ptr web = WriterInterface::ptr(new WebWriter);

    // create database queue
    QueueInterface::ptr db_queue = QueueInterface::ptr(new Queue());
    // create web queue
    QueueInterface::ptr web_queue = QueueInterface::ptr(new Queue());
    // append hardware thread
    threads_.emplace_back(Application::Thread(new std::jthread(std::bind(&CollectorInterface::collect, hardware, db_queue))));
    // append data base queue
    threads_.emplace_back(Application::Thread(new std::jthread(std::bind(&CollectorInterface::collect, database, web_queue))));

    // write database from database queue 
    threads_.emplace_back(Application::Thread(new std::jthread(std::bind(&WriterInterface::write, database, db_queue))));
    // write web req to web writer
    threads_.emplace_back(Application::Thread(new std::jthread(std::bind(&WriterInterface::write, web, web_queue))));
}

void Application::init_config(std::vector<ConfigInterface::ptr> vec) {
    // load all config
    for (auto iter : vec)
        iter->load_from_file(iter->get_config_file());
}

void Application::stop() {
    for (auto iter : threads_) {
        iter->request_stop();
    }
}

}