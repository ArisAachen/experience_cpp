#ifndef __EXPERIENCE_SRC_DATA_H__
#define __EXPERIENCE_SRC_DATA_H__

#include "define.h"
#include "define.pb.h"

#include <map>
#include <memory>

#include <core/dbus/bus.h>
#include <core/dbus/types/object_path.h>
#include <mutex>

namespace experience {

class AppCollector : public CollectorInterface {
public:
    typedef std::shared_ptr<AppCollector> ptr;

    /**
     * @brief ollect message to queue
     * @param[in] que collect queue
     */
    virtual void collect(QueueInterface::ptr que) override;

    /**
     * @brief handle request result
     * @param[in] result req result
     */
    virtual void handler(ReqResult::ptr result) override;

private:
    typedef core::dbus::Bus Bus;
    typedef core::dbus::Bus::Ptr BusPtr;
    typedef core::dbus::types::ObjectPath ObjectPath;
    // typedef std::shared_ptr<define::AppEntry> App;

private:
    /**
     * @brief monitor entry app
     * @param[in] que write queue
     * @param[in] path entry path
     */
    void monitor_entry(QueueInterface::ptr que, ObjectPath & path);

    /**
     * @brief pop request app
     * @param[in] que queue 
     * @param[in] app app msg
     */
    void pop_req(QueueInterface::ptr que, define::AppEntry & app);

    /**
     * @brief add entry to map
     * @param[in] pah entry path
     * @param[in] entry entry obj
     */
    bool add_entry(ObjectPath & pah, define::AppEntry& entry);

    /**
     * @brief 
     * @param pah 
     */
    bool del_entry(ObjectPath & pah);
private:
    /// bus 
    BusPtr bus;
    /// mutex
    std::mutex mutex_;
    /// save appentry message
    std::map<ObjectPath, define::AppEntry> apps_;
};






}

#endif