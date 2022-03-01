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


#ifndef __EXPERIENCE_SRC_INTERFACE_H__
#define __EXPERIENCE_SRC_INTERFACE_H__

#include "define.h"

#include <memory>
#include <string>
#include <queue>

namespace experience {

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
     * @param url connect path
     */
    virtual void connect(const std::string & url);

    /**
     * @brief disconnect from server
     */
    virtual void disconnect();

    /**
     * @brief write queue data to writer
     * @param que request queue
     */
    virtual void write(QueueInterface::ptr que);
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