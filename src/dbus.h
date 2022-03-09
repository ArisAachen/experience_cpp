#ifndef __EXPERIENCE_SRC_DBUS_H__
#define __EXPERIENCE_SRC_DBUS_H__


#include <cstdint>
#include <map>
#include <string>
#include <chrono>
#include <tuple>
#include <vector>

#include <core/dbus/bus.h>
#include <core/dbus/service.h>
#include <core/dbus/traits/service.h>
#include <core/dbus/types/object_path.h>

namespace experience {
class IExperienceService {
protected:
    struct Enable {
        typedef IExperienceService Interface;
        inline static const std::string name() {
            return "Enable";
        }
        inline static const std::chrono::milliseconds default_timeout()
        {
            return std::chrono::seconds{1};
        }
    };

    struct IsEnabled {
        typedef IExperienceService Interface;
        inline static const std::string name() {
            return "IsEnabled";
        }
        inline static const std::chrono::milliseconds default_timeout()
        {
            return std::chrono::seconds{1};
        }
    };

public:
    virtual~IExperienceService() = default;
    // // enable method
    // virtual void enable(bool enabled) = 0;
    // // get enable state
    // virtual bool is_enabled() = 0;
};
}


namespace core {

struct DeviceManager {
    // method
    struct getInfo {
        typedef DeviceManager Interface;
        static const std::string name() {
            return "getInfo";
        }
        inline static const std::chrono::milliseconds default_timeout() { return std::chrono::seconds{1}; }
    };

        // method
    struct refreshInfo {
        static const std::string name() {
            return "refreshInfo";
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

struct DBus {
    struct GetNameOwner {
        typedef DBus Interface;
        static const std::string name() {
            return "GetNameOwner";
        }
        inline static const std::chrono::milliseconds default_timeout() { return std::chrono::seconds{1}; }
    };

    struct NameHasOwner {
        typedef DBus Interface;
        static const std::string name() {
            return "NameHasOwner";
        }
        inline static const std::chrono::milliseconds default_timeout() { return std::chrono::seconds{1}; }
    };
};

struct Dock {
    struct Properties {
        // entries
        struct Entries {
            inline static std::string name()
            {
                return "Entries";
            };
            typedef Dock Interface;
            typedef std::vector<dbus::types::ObjectPath> ValueType;
            static const bool readable = true;
            static const bool writable = false;
        };
    };

    struct Signals {
        struct EntryAdded {
            inline static std::string name() {
                return "EntryAdded";
            }
            typedef Dock Interface;            
            typedef std::tuple<dbus::types::ObjectPath, int32_t> ArgumentType;
        };

        struct EntryRemoved {
            inline static std::string name() {
                return "EntryRemoved";
            }
            typedef Dock Interface;
            typedef std::string ArgumentType;
        };
    };

    struct Entry {
        struct Properties {
            struct WindowInfos {
                inline static std::string name() {
                    return "WindowInfos";
                }
                typedef Entry Interface;
                typedef std::map<uint32_t, std::tuple<std::string, bool>> ValueType;
                static const bool readable = true;
                static const bool writable = false;
            };
            struct DesktopFile {
                inline static std::string name() {
                    return "DesktopFile";
                }
                typedef Entry Interface;
                typedef std::string ValueType;
                static const bool readable = true;
                static const bool writable = false;                
            };
            struct Name {
                inline static std::string name() {
                    return "Name";
                }
                typedef Entry Interface;
                typedef std::string ValueType;
                static const bool readable = true;
                static const bool writable = false;                
            };
        };
    };
};


struct NetworkManager {
    struct Properties {
        struct Connectivity {
            inline static std::string name()
            {
                return "Connectivity";
            };
            typedef NetworkManager Interface;
            typedef uint32_t ValueType;
            static const bool readable = true;
            static const bool writable = false;                  
        };
    };
};

}

namespace core {
namespace dbus {
namespace traits {

template<>
struct Service<core::DeviceManager> {
    inline static const std::string interface_name()
    {
        return "com.deepin.devicemanager";
    }    
};

template<>
struct Service<core::DBus> {
    inline static const std::string interface_name() 
    {
        return "org.freedesktop.DBus";        
    }
};

template<>
struct Service<core::Dock> {
    inline static const std::string& interface_name() 
    {
        static const std::string s {
            "com.deepin.dde.daemon.Dock"
        };
        return  s;
    }
};

template<>
struct Service<core::Dock::Entry> {
    inline static const std::string interface_name() 
    {
        return "com.deepin.dde.daemon.Dock.Entry";        
    }    
};

template<>
struct Service<core::NetworkManager> {
    inline static const std::string interface_name() {
        return  "org.freedesktop.NetworkManager";
    }
};

template<>
struct Service<experience::IExperienceService> {
    inline static const std::string interface_name()
    {
        return "com.deepin.userexperience.Daemon";
    }    
};

}
}
}


#endif
