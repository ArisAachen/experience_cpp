#ifndef __EXPERIENCE_SRC_DBUS_H__
#define __EXPERIENCE_SRC_DBUS_H__

#include <string>
#include <chrono>

#include <core/dbus/service.h>
#include <core/dbus/traits/service.h>
#include <core/dbus/types/object_path.h>



namespace core {

struct DeviceManager {
    // method
    struct getInfo {
        typedef DeviceManager Interface;
        static const std::string& name() {
            static const std::string s 
            {
                "getInfo"
            };
            return s;
        }
        inline static const std::chrono::milliseconds default_timeout() { return std::chrono::seconds{1}; }
    };

        // method
    struct refreshInfo {
        static const std::string& name() {
            static const std::string s 
            {
                "refreshInfo"
            };
            return s;
        }
        typedef DeviceManager Interface;
        inline static const std::chrono::milliseconds default_timeout() { return std::chrono::seconds{1}; }
    };

    // signal
    struct Signals {
        struct update {
            inline static std::string name() 
            {
                return "update";
            }
            typedef DeviceManager Interface;
        };
    };
};




}

namespace core {
namespace dbus {
namespace traits {




template<>
struct Service<core::DeviceManager> {
    inline static const std::string& interface_name()
    {
        static const std::string s
        {
            "com.deepin.devicemanager"
        };
        return s;
    }    
};

}
}
}


#endif
