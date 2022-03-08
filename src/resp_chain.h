#ifndef __EXPERIENCE_SRC_CONTROL_H__
#define __EXPERIENCE_SRC_CONTROL_H__

#include "define.h"

#include <cstdint>

#include <core/dbus/bus.h>
#include <core/dbus/types/object_path.h>


namespace experience {

// NetworkRespChain use to control write op according to network state
class NetworkRespChain : public RespChainInterface {
public:
    /**
     * @brief init network resp chain
     */
    virtual void init() override;

private:
    typedef core::dbus::Bus Bus;
    typedef core::dbus::Bus::Ptr BusPtr;
    typedef core::dbus::types::ObjectPath ObjectPath;    
};


// ExplanRespChain use to control write op according to experience plan enable state
class ExplanRespChain : public RespChainInterface {
public:
    
};









}

#endif