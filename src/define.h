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

#include <cstddef>
#include <functional>
#include <memory>
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

// ============================================================== string define ============================================================== //

const std::string experience_dir = "/var/public/deepin-user-experience";
const std::string hardware_file = experience_dir + "/" + "hardware";
const std::string database_fle = experience_dir + "/" + "exp.db";
const std::string database_table = "exp";

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

// ============================================================== interface define ============================================================== //

class CryptorInterface {
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
class ConfigInterface {
public:
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
};

// QueueInterface use to store queue message
class QueueInterface {
public:
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

protected:
    std::queue<ReqMessage::ptr> queue_ ;
};

// CollectorInterface collect info to queue, 
// and handle result once info write to writer
class CollectorInterface {
public:
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

class Controller {

};


class WriterInterface {
public:
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
     * @brief write queue data to writer
     * @param[in] que request queue
     */
    virtual void write(QueueInterface::ptr que) = 0;
};

class ModuleCfgCor : public ConfigInterface, public CollectorInterface {
public:
    typedef std::shared_ptr<ModuleCfgCor> ptr;
};

class ModuleWtrCor : public CollectorInterface, public WriterInterface {
public:
    typedef std::shared_ptr<ModuleWtrCor> ptr;
};

}

#endif