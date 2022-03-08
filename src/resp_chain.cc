#include "resp_chain.h"
#include "dbus.h"
#include "log.h"
#include "utils.h"

#include <cstdint>
#include <memory>
#include <mutex>

#include <core/dbus/signal.h>
#include <core/dbus/asio/executor.h>
#include <core/dbus/interfaces/properties.h>
#include <core/dbus/service.h>
#include <core/dbus/property.h>
#include <core/dbus/traits/service.h>
#include <core/dbus/well_known_bus.h>


namespace experience {


NetworkRespChain::NetworkRespChain() {

}



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
        return;
    }
    EXPERIENCE_DEBUG("network manager init offline, dont allow to post message");
}





}