/*
 * Copyright (C) 2014 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     ArisAachen <chenyunxiong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __EXPERIENCE_SRC_DEFINE_H__
#define __EXPERIENCE_SRC_DEFINE_H__

#include <condition_variable>
#include <cstddef>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <queue>
#include <vector>

namespace experience {

// ============================================================== int define ============================================================== //

enum class TidTyp {
    GeneralTid     = 1,
	SystemInfoTid  = 1000,
	CheckUpdateTid = 1001,
	LoginTid       = 1002,
	LogoutTid      = 1003,
	AppOpenTid     = 1004,
	AppCloseTid    = 1005
};

enum class ReqResultCode {
    /// write success
    WriteResultSuccess,
    /// write failed, need delete data
    WriteResultUnsavedFailed,
    /// write failed, dont need delete data
    WriteResultSavedFailed,
};

const int aes_key_length = 32;
const int aes_iv_length = 16;

// ============================================================== string define ============================================================== //

const std::string experience_dir = "/var/public/deepin-user-experience";
const std::string hardware_file = experience_dir + "/" + "hardware";
const std::string database_fle = experience_dir + "/" + "exp.db";
const std::string database_table = "exp";

const std::string etc_dir = "/etc";
const std::string zone_file = etc_dir + "/" + "timezone";
const std::string system_exlan_file = etc_dir + "/" + "deepin/deepin-user-experience";
const std::string machine_id_file = etc_dir + "/"  + "machine-id";
const std::string apt_token_file = etc_dir + "/" + "apt" + "/" + "apt.conf.d" + "/" + "99lastore-token.conf";

const std::string post_aid        = "uospc2";
const std::string post_content_type = "application/json";
const std::string post_unification = "v2/report/unification";
const std::string post_url_first  = "https://bdapapi1.uniontech.com";
const std::string post_url_second = "https://bdapapi2.uniontech.com";
const std::string post_url_third  = "https://bdapapi3.uniontech.com";

// ============================================================== dbus define ============================================================== //

const std::string device_manager_dbus_path = "/com/deepin/devicemanager";

// ============================================================== message define ============================================================== //

struct CryptResult {
    typedef std::shared_ptr<CryptResult> ptr;
    /// origin data
    std::string origin;
    /// crypt key
    std::string key;
    /// crypt result
    std::string result;
};

struct ReqResult {
    typedef std::shared_ptr<ReqResult> ptr;

    /// decoder
    std::function<std::string(CryptResult::ptr msg)> decode;
    /// code
    ReqResultCode code;
    /// origin
    std::string origin;
    /// msg
    std::string msg;
};

struct ReqMessage {
    typedef std::shared_ptr<ReqMessage> ptr;

    /// encoder
    std::function<CryptResult::ptr(const std::string & msg)> encode;
    /// call back
    std::function<void(ReqResult::ptr)> call_back;
    /// message
    std::vector<std::string> vec;
    /// tid
    TidTyp tid;
};

struct Data {
    typedef std::shared_ptr<Data> ptr;
    // data type
    int typ;
    // data 
    std::string data;
};

// ============================================================== system info ============================================================== //

enum class SysModuleIndex  {
    CpuModuleIndex,
    BoardModuleIndex,
    MemoryModuleIndex,
    DiskModuleValue,
    SmartDiskModuleIndex,
    GpuModuleIndex,
    NetModuleIndex,
    EtherModuleIndex,
};

// hardware info 
struct HardwareMsg {
    typedef std::shared_ptr<HardwareMsg> ptr;
    std::string model;
    std::string id;
};

// ============================================================== interface define ============================================================== //

struct CryptorInterface {
public:
    typedef std::shared_ptr<CryptorInterface> ptr;
    /**
     * @brief encode string to data 
     * @param[in] msg encode msg
     */
    virtual CryptResult::ptr encode(const std::string & msg) = 0;

    /**
     * @brief decode data to string
     * @param[in] msg decode msg
     */
    virtual std::string decode(CryptResult::ptr msg) = 0;
};

// ConfigInterface read and write config file
struct ConfigInterface {
    typedef std::shared_ptr<ConfigInterface> ptr;
    /**
     * @brief save config to file
     * @param[in] filename file name
     */
    virtual void save_to_file(const std::string & filename) = 0;

    /**
     * @brief load config from file
     * @param[in] filename file name
     */
    virtual void load_from_file(const std::string & filename) = 0;

    /**
     * @brief indicate where module need update
     */
    virtual bool need_update() = 0;

    /**
     * @brief Get the config file object
     */
    virtual const std::string get_config_file() = 0;
};

// QueueInterface use to store queue message
struct QueueInterface {
    typedef std::shared_ptr<QueueInterface> ptr;
    /**
     * @brief push request message 
     * @param[in] req rvalue request message
     */
    virtual void push(ReqMessage::ptr req) = 0;

    /**
     * @brief pop request message
     */
    virtual ReqMessage::ptr pop() = 0;

    /**
     * @brief clear queue
     */
    virtual void clear() = 0;

protected:
    std::queue<ReqMessage::ptr> queue_ ;
};

// CollectorInterface collect info to queue, 
// and handle result once info write to writer
struct CollectorInterface {
    typedef std::shared_ptr<CollectorInterface> ptr;
    /**
     * @brief ollect message to queue
     * @param[in] que collect queue
     */
    virtual void collect(QueueInterface::ptr que) = 0;

    /**
     * @brief handle request result
     * @param[in] result req result
     */
    virtual void handler(ReqResult::ptr result) = 0;
};

struct RespChainInterface {
    typedef std::shared_ptr<RespChainInterface> ptr;

    /**
     * @brief init resp chain
     */
    virtual void init() = 0;

    /**
     * @brief Set the block object
     * @param[in] block block state
     */
    virtual void set_block(bool block);

    /**
     * @brief Get the block object
     */
    virtual void block();

    /**
     * @brief Set the next chain object
     * @param[in] next next resp chain
     */
    virtual void set_next_chain(RespChainInterface::ptr next);

protected:
    /// block state
    bool blocked_ {true};
    /// block state mutex
    std::mutex mutex_;
    // wait condition
    std::condition_variable nonblock_cond_ ; 
    /// next respon chain
    RespChainInterface::ptr next_;
};

struct WriterInterface {
    typedef std::shared_ptr<WriterInterface> ptr;
    /**
     * @brief try to connect to url path
     * @param[in] url connect path
     */
    virtual void connect(const std::string & url) = 0;

    /**
     * @brief disconnect from server
     */
    virtual void disconnect() = 0;

    /**
     * @brief Set the resp chain object
     * @param[in] resp response rule chain
     */
    virtual void set_resp_chain(RespChainInterface::ptr resp) = 0;

    /**
     * @brief write queue data to writer
     * @param[in] que request queue
     */
    virtual void write(QueueInterface::ptr que) = 0;

    /**
     * @brief Get the remote object
     */
    virtual const std::string get_remote() = 0;
};

struct ModuleCfgCor : public ConfigInterface, public CollectorInterface {
    typedef std::shared_ptr<ModuleCfgCor> ptr;
};

struct ModuleWtrCor : public CollectorInterface, public WriterInterface {
    typedef std::shared_ptr<ModuleWtrCor> ptr;
};

struct ModuleCfgResp : public ConfigInterface, public RespChainInterface {
    typedef std::shared_ptr<ModuleCfgResp> ptr;
};

struct ModuleWtrResp : public WriterInterface, public RespChainInterface {
    typedef std::shared_ptr<ModuleWtrResp> ptr;
};

}

#endif