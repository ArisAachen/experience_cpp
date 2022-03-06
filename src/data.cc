#include "data.h"
#include "dbus.h"
#include "define.h"
#include "define.pb.h"
#include "log.h"
#include "utils.h"

#include <cstdint>
#include <ctime>
#include <functional>
#include <sstream>
#include <string>
#include <tuple>
#include <unistd.h>
#include <utility>
#include <vector>
#include <mutex>
#include <random>
#include <exception>
#include <memory>

#include <core/dbus/bus.h>
#include <core/dbus/object.h>
#include <core/dbus/property.h>
#include <core/dbus/service.h>


#include <core/dbus/bus.h>
#include <core/dbus/service.h>
#include <core/dbus/signal.h>
#include <core/dbus/traits/service.h>
#include <core/dbus/types/object_path.h>
#include <core/dbus/types/struct.h>
#include <core/dbus/interfaces/properties.h>
#include <core/dbus/asio/executor.h>
#include <core/dbus/object.h>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/constants.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim_all.hpp>


namespace experience {


AppCollector::AppCollector() {

}

// collect data
void AppCollector::collect(QueueInterface::ptr que) {
    // wait dde dock service
    DBusUtils::wait_session_dbus("com.deepin.dde.daemon.Dock");

    try {
        // create session bus
        bus = std::make_shared<Bus>(core::dbus::WellKnownBus::session);
        bus->install_executor(core::dbus::asio::make_executor(bus));
        auto dock_service = core::dbus::Service::use_service(bus, core::dbus::traits::Service<core::Dock>::interface_name());
        auto dock_obj = dock_service->object_for_path(ObjectPath("/com/deepin/dde/daemon/Dock"));
        auto entries = dock_obj->get_property<core::Dock::Properties::Entries>()->get();

        // monitor signal
        auto dock_added_signal = dock_obj->get_signal<core::Dock::Signals::EntryAdded>();
        dock_added_signal->connect([&que, this](const core::Dock::Signals::EntryAdded::ArgumentType& args) {
            // get dock path
            auto dock_path = std::get<0>(args);
            this->monitor_entry(que, dock_path);
        });

        auto dock_removed_signal = dock_obj->get_signal<core::Dock::Signals::EntryRemoved>();
        dock_removed_signal->connect([&](const core::Dock::Signals::EntryRemoved::ArgumentType& arg) {
            // lock
            std::lock_guard<std::mutex> lock(mutex_);
            // try to find
            auto iter = apps_.find(arg);
            if (iter == apps_.end()) {
                EXPERIENCE_FMT_DEBUG("receive %s dock entry removed signal, but not store last", arg.c_str());
                return ;
            }
            EXPERIENCE_FMT_DEBUG("receive dock entry removed signal, app: %s", iter->second.app().c_str());

        });

        

    } catch (std::exception & e) {

    }
}

void AppCollector::handler(ReqResult::ptr result) {

}

// monitor entry
void AppCollector::monitor_entry(QueueInterface::ptr que, ObjectPath & path) {
    
    // create interface name
    auto entry_service = core::dbus::Service::use_service(bus, core::dbus::traits::Service<core::Dock::Entry>::interface_name());
    auto entry_obj = entry_service->object_for_path(path);
    // get entry info
    auto app_name = entry_obj->get_property<core::Dock::Entry::Properties::Name>()->get();
    auto desktop_file = entry_obj->get_property<core::Dock::Entry::Properties::DesktopFile>()->get();
    // run to get 
    std::string cmd = std::string("dpkg") + std::string(" ") + std::string("-S") + std::string(" ") + desktop_file;
    std::string cmd_out = StringUtils::execute_cmd(cmd);
    // trim space
    boost::algorithm::trim_all(cmd_out);
    // create vec
    std::vector<std::string> vec;
    // split command
    boost::algorithm::split(vec, cmd_out, boost::is_any_of(":"), boost::token_compress_on);
    // check size
    if (vec.size() < 2) {
        return;
    }
    // save app info
    define::AppEntry app_info;
    app_info.set_app(app_name);
    app_info.set_time(time(nullptr));
    app_info.set_pkg(vec.at(1));
    auto property_changed_signal = entry_obj->get_signal<core::dbus::interfaces::Properties::Signals::PropertiesChanged>();
    property_changed_signal->connect([this, &path, &app_info, &que](const std::tuple<std::string, std::map<std::string, core::dbus::types::Variant>, 
            std::vector<std::string>> args) {
        // search name
        auto property_map = std::get<1>(args);
        auto iter = property_map.find(core::Dock::Entry::Properties::WindowInfos::name());
        if (iter == property_map.end())
            return;
        // std::map<uint32_t, std::tuple<std::string, bool>> decode;
        auto decode = iter->second.as<std::map<uint32_t, std::tuple<std::string, bool>>>();
        // check if is empty
        if (decode.size() == 0 && this->del_entry(path)) {
            app_info.set_tid(int(TidTyp::AppCloseTid));
            pop_req(que, app_info);
        } else if (decode.size() != 0 && this->add_entry(path, app_info)) {
            app_info.set_tid(int(TidTyp::AppOpenTid));
            pop_req(que, app_info);
        }
    });

    auto info = entry_obj->get_property<core::Dock::Entry::Properties::WindowInfos>()->get();
    if (info.size() != 0 && add_entry(path, app_info)) {
        app_info.set_tid(int(TidTyp::AppOpenTid));
        pop_req(que, app_info);       
    } 
}

void AppCollector::pop_req(QueueInterface::ptr que, define::AppEntry & app) {
    // create request message
    ReqMessage::ptr req;
    req->tid = TidTyp(app.tid());
    req->call_back = std::bind(&AppCollector::handler, this, std::placeholders::_1);
    std::string info;
    app.SerializeToString(&info);
    req->vec.emplace_back(info);
    que->push(req);
}

bool AppCollector::add_entry(ObjectPath & path, define::AppEntry& entry) {
    std::lock_guard<std::mutex> lock(mutex_);
    // search for app
    auto iter = apps_.find(path);
    // already exist, should not add again
    if (iter != apps_.end())
        return false;
    // insert to map
    apps_.insert(std::make_pair(path, entry));
    return true;
}

bool AppCollector::del_entry(ObjectPath & path) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto iter = apps_.find(path);
    if (iter == apps_.end())
        return false;
    apps_.erase(path);
    return true;
}

}