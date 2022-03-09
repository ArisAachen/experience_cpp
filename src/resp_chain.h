#ifndef __EXPERIENCE_SRC_CONTROL_H__
#define __EXPERIENCE_SRC_CONTROL_H__

#include "dbus.h"
#include "define.h"


#include <cstdint>

#include <core/dbus/bus.h>
#include <core/dbus/skeleton.h>
#include <core/dbus/types/object_path.h>
#include <core/dbus/object.h>


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
class ExplanRespChain : public ModuleCfgResp, public core::dbus::Skeleton<IExperienceService> {
public:
    typedef std::shared_ptr<ExplanRespChain> Ptr;

    /**
     * @brief Construct a new Explan Resp Chain object
     */
    ExplanRespChain();
    /**
     * @brief save config to file
     * @param[in] filename file name
     */
    virtual void save_to_file(const std::string & filename) override;

    /**
     * @brief load config from file
     * @param[in] filename file name
     */
    virtual void load_from_file(const std::string & filename) override;

    /**
     * @brief indicate where module need update
     */
    virtual bool need_update() override;

    /**
     * @brief Get the config file object
     */
    virtual const std::string get_config_file() override;

    /**
     * @brief init resp chain
     */
    virtual void init() override;

private:
    /**
     * @brief set enabled state
     * @param[in] enabled enable state
     */
    void enable(const core::dbus::Message::Ptr& msg);

    /**
     * @brief get enable state
     */
    void is_enabled(const core::dbus::Message::Ptr& msg);

private:
    /// export object
    core::dbus::Object::Ptr obj;
};









}

#endif