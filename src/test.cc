#include "dbus.h"

#include <core/dbus/bus.h>
#include <core/dbus/object.h>
#include <core/dbus/property.h>
#include <core/dbus/service.h>
#include <core/dbus/asio/executor.h>

#include <string>
#include <vector>
#include <boost/algorithm/string.hpp>



core::dbus::Bus::Ptr the_system_bus() {
    // get system bus
    static auto system_bus = std::make_shared<core::dbus::Bus>(core::dbus::WellKnownBus::system);
    return system_bus;
}

void test() {

    auto sys = the_system_bus();
    sys->install_executor(core::dbus::asio::make_executor(sys));
    auto device_manager = core::dbus::Service::use_service(sys, core::dbus::traits::Service<core::DeviceManager>::interface_name());
    auto manager_obj = device_manager->object_for_path(core::dbus::types::ObjectPath(""));
}