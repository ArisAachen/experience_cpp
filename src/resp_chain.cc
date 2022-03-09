#include "resp_chain.h"
#include "dbus.h"
#include "log.h"
#include "utils.h"


#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include <signal.h>
#include <bits/types/sigset_t.h>
#include <core/dbus/signal.h>
#include <core/dbus/message.h>
#include <core/dbus/asio/executor.h>
#include <core/dbus/interfaces/properties.h>
#include <core/dbus/service.h>
#include <core/dbus/announcer.h>
#include <core/dbus/property.h>
#include <core/dbus/traits/service.h>
#include <core/dbus/well_known_bus.h>
#include <core/dbus/skeleton.h>
#include <core/dbus/bus.h>
#include <core/dbus/types/object_path.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree_fwd.hpp>



namespace experience {

void NetworkRespChain::init() {
    // wait for network manager service
    DBusUtils::wait_system_dbus("org.freedesktop.NetworkManager");

    // create system bus
    auto bus = std::make_shared<Bus>(core::dbus::WellKnownBus::system);
    bus->install_executor(core::dbus::asio::make_executor(bus));
    // create network manager obj
    auto nm_service = core::dbus::Service::use_service(bus, core::dbus::traits::Service<core::NetworkManager>::interface_name());
    auto nm_obj = nm_service->object_for_path(ObjectPath("/org/freedesktop/NetworkManager"));
    // check signal
    auto property_changed_signal = nm_obj->get_signal<core::dbus::interfaces::Properties::Signals::PropertiesChanged>();
    property_changed_signal->connect([this](const std::tuple<std::string, std::map<std::string, core::dbus::types::Variant>, 
        std::vector<std::string>> args) {
        auto property_map = std::get<1>(args);
        // check if is connectivity property changed
        auto iter = property_map.find("Connectivity");
        if (iter == property_map.end())
            return;
        // convert
        auto connectivity = iter->second.as<uint32_t>();
        if (connectivity == 4) {
            set_block(false);
            EXPERIENCE_DEBUG("network manager set online now, allow to post message");
            return;
        }
        EXPERIENCE_DEBUG("network manager set online offline, dont allow to post message");
        set_block(true);
    });
    // get first property
    auto connectivity = nm_obj->get_property<core::NetworkManager::Properties::Connectivity>()->get();
    if (connectivity == 4) {
        EXPERIENCE_DEBUG("network manager init online, allow to post message");
        set_block(false);
    } else {
        EXPERIENCE_DEBUG("network manager init offline, dont allow to post message");
    }
    // sig set
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    int sig;
    sigwait(&set, &sig);
}

ExplanModule::ExplanModule(const core::dbus::Bus::Ptr& bus) :
// create session bus
core::dbus::Skeleton<IExperienceService>(bus)
{   

}

// save experience plan to file
void ExplanModule::save_to_file(const std::string &filename) {
    // convert to node, 
    boost::property_tree::ptree node;
    node.put<bool>("ExperiencePlan.ExperienceState", !blocked_);
    // write config to ini file
    boost::property_tree::ini_parser::write_ini(filename, node);
    EXPERIENCE_FMT_INFO("write experien state success, state: %d", !blocked_);
}

// load from file
void ExplanModule::load_from_file(const std::string &filename) {
    // convert to node, 
    boost::property_tree::ptree node;
    node.get<bool>("ExperiencePlan.ExperienceState", !blocked_);
    SystemInfo::set_experience_enable(!blocked_);
    EXPERIENCE_FMT_INFO("load experience state success, state: %d", !blocked_);
}

bool ExplanModule::need_update() {
    return false;
}

// get config file
const std::string ExplanModule::get_config_file() {
    return system_exlan_file;
}

void ExplanModule::init() {
    // export on session bus
    obj_ = core::dbus::Object::Ptr(access_service()->add_object_for_path(core::dbus::types::ObjectPath("/com/deepin/userexperience/Daemon")));
    // install method handle
    obj_->install_method_handler<IExperienceService::Enable>(std::bind(&ExplanModule::enable, this, std::placeholders::_1));
    obj_->install_method_handler<IExperienceService::IsEnabled>(std::bind(&ExplanModule::is_enabled, this, std::placeholders::_1));
    auto bus = std::make_shared<core::dbus::Bus>(core::dbus::WellKnownBus::session);
    bus->install_executor(core::dbus::asio::make_executor(bus));
    auto explan_service = core::dbus::announce_service_on_bus<IExperienceService, ExplanModule>(bus);

    // sig set
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    int sig;
    sigwait(&set, &sig);
}

void ExplanModule::enable(const core::dbus::Message::Ptr& msg) {
    // set enabled state
    bool enabled;
    msg->reader() >> enabled;
    set_block(!enabled);
}

void ExplanModule::is_enabled(const core::dbus::Message::Ptr& msg) {
    auto reply = core::dbus::Message::make_method_return(msg);
    reply->writer() << !blocked_;
    access_bus()->send(reply);
}


}