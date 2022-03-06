#include "data.h"
#include "dbus.h"
#include "define.h"
#include "define.pb.h"
#include "log.h"
#include "utils.h"

#include <ctime>
#include <functional>
#include <string>
#include <utility>
#include <vector>
#include <mutex>
#include <random>
#include <exception>
#include <memory>

#include <core/dbus/bus.h>
#include <core/dbus/service.h>
#include <core/dbus/signal.h>
#include <core/dbus/traits/service.h>
#include <core/dbus/types/object_path.h>
#include <core/dbus/asio/executor.h>
#include <core/dbus/object.h>
#include <core/dbus/interfaces/properties.h>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/constants.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim_all.hpp>


namespace experience {


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
        // monitor signal
        auto dock_added_signal = dock_obj->get_signal<core::Dock::Signals::EntryAdded>();
        dock_added_signal->connect([&](core::Dock::Signals::EntryAdded::ArgumentType& args) {
            // get dock path
            auto dock_path = std::get<0>(args);
            // create interface name
            auto entry_service = core::dbus::Service::use_service(bus, core::dbus::traits::Service<core::Dock::Entry>::interface_name());
            auto entry_obj = entry_service->object_for_path(dock_path);
            auto info = dock_obj->get_property<core::Dock::Entry::Properties::WindowInfos>()->get();
            
        });

        auto dock_removed_signal = dock_obj->get_signal<core::Dock::Signals::EntryRemoved>();
        dock_removed_signal->connect([&](core::Dock::Signals::EntryRemoved::ArgumentType& arg) {
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

// monitor entry
void AppCollector::monitor_entry(QueueInterface::ptr que, ObjectPath & path) {
    
    // create interface name
    auto entry_service = core::dbus::Service::use_service(bus, core::dbus::traits::Service<core::Dock::Entry>::interface_name());
    auto entry_obj = entry_service->object_for_path(path);
    // get entry info
    auto app_name = entry_obj->get_property<core::Dock::Entry::Properties::Name>()->get();
    auto desktop_file = entry_obj->get_property<core::Dock::Entry::Properties::DesktopFile>()->get();
    // run to get 
    std::string cmd = "dpkg" + " " + "-S" + " " + desktop_file;
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
    property_changed_signal->connect([this, &path, &app_info, &que](core::dbus::interfaces::Properties::Signals::PropertiesChanged::ArgumentType& args) {
        // search name
        auto property_map = std::get<1>(args);
        auto iter = property_map.find(core::Dock::Entry::Properties::WindowInfos::name());
        if (iter == property_map.end())
            return;
        // check if is empty
        if (iter.second == nullptr && this->del_entry(path)) {
            app_info.set_tid(int(TidTyp::AppCloseTid));
            pop_req(que, app_info);
        } else if (iter.second != nullptr && this->add_entry(path, app_info)) {
            app_info.set_tid(int(TidTyp::AppOpenTid));
            pop_req(que, app_info);
        }
    });

    auto info = entry_obj->get_property<core::Dock::Entry::Properties::WindowInfos>()->get();

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