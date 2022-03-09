#include "application.h"
#include "define.h"
#include "log.h"
#include "config.h"
#include "resp_chain.h"
#include "utils.h"
#include "writer.h"
#include "data.h"

#include <exception>
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

#include <unistd.h>
#include <core/dbus/bus.h>
#include <core/dbus/well_known_bus.h>

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
    EXPERIENCE_INFO("application start");

    // create database queue
    QueueInterface::ptr db_queue = QueueInterface::ptr(new Queue());
    // create web queue
    QueueInterface::ptr web_queue = QueueInterface::ptr(new Queue());

    // config interface vector
    std::vector<ConfigInterface::ptr> cfg_vec;
    // writer vector
    std::vector<WriterInterface::ptr> writer_vec;
    // collector vector
    std::vector<std::tuple<std::string, CollectorInterface::ptr, QueueInterface::ptr>> col_vec;
    // resp vector 
    std::vector<RespChainInterface::ptr> resp_vec;

    // hardware config and collector
    ModuleCfgCor::ptr hardware = ModuleCfgCor::ptr(new HardModule);
    cfg_vec.push_back(hardware);
    col_vec.emplace_back(std::make_tuple("hardware collector", hardware, db_queue));
    // database collector and writer
    ModuleWtrCor::ptr database = ModuleWtrCor::ptr (new DBModule);
    writer_vec.push_back(database);
    col_vec.emplace_back(std::make_tuple("database collector", database, web_queue));
    // explan config and respon 
    ModuleCfgResp::ptr explan = ModuleCfgResp::ptr(new ExplanModule(core::dbus::Bus::Ptr(
        std::make_shared<core::dbus::Bus>(core::dbus::WellKnownBus::session))));
    cfg_vec.push_back(explan);
    resp_vec.push_back(explan);
    // web writer and response
    ModuleWtrResp::ptr wtr_resp = ModuleWtrResp::ptr(new ModuleWeb);
    writer_vec.push_back(wtr_resp);
    resp_vec.push_back(wtr_resp);
    
    // app collector
    CollectorInterface::ptr app = CollectorInterface::ptr(new AppCollector);
    col_vec.emplace_back(std::make_tuple("app dock collector", app, db_queue));

    // network response
    RespChainInterface::ptr network = RespChainInterface::ptr(new NetworkRespChain);
    resp_vec.push_back(network);
    // set response relation
    explan->set_next_chain(network);
    network->set_next_chain(wtr_resp);

    // begin to init all module
    init_config(cfg_vec);
    init_resp(resp_vec);
    init_writer(writer_vec);
    init_collector(col_vec);
}

void Application::init_config(std::vector<ConfigInterface::ptr> vec) {
    // load all config
    for (auto iter : vec)
        iter->load_from_file(iter->get_config_file());
}

void Application::init_writer(std::vector<WriterInterface::ptr> vec) {
    for (auto iter : vec) 
        iter->connect(iter->get_remote());
}

void Application::init_collector(std::vector<std::tuple<std::string, CollectorInterface::ptr, QueueInterface::ptr>> vec) {
    // init writer
    for (auto iter : vec) {
        // get collector
        auto col = std::get<1>(iter);
        auto que = std::get<2>(iter);
        // run thread
        threads_.emplace_back(Application::Thread(new std::jthread(std::bind(&CollectorInterface::collect, col, que))));
    }
}

void Application::init_resp(std::vector<RespChainInterface::ptr> vec) {
    for (auto iter : vec)
        threads_.emplace_back(Application::Thread(new std::jthread(std::bind(&RespChainInterface::init, iter))));
}


void Application::stop() {
    for (auto iter : threads_) {
        iter->request_stop();
    }
}

}