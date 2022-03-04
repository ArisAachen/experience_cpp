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

#ifndef __EXPERIENCE_SRC_CONFIG_H__
#define __EXPERIENCE_SRC_CONFIG_H__

#include "define.h"
#include "define.pb.h"

#include <memory>

namespace experience {

// type name T is protobuf type
template <typename T>
class Module : public ModuleCfgCor {
public:
    // module ptr
    typedef std::shared_ptr<Module> ptr;
    // protobuf share ptr
    // typedef std::shared_ptr<T> proto;
    /**
     * @brief Construct a new Module object
     */
    Module();

    /**
     * @brief Destroy the virtual Module object
     */
    virtual~Module();

    /**
     * @brief save config message to file
     * @param[in] filename save to file
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
     * @brief ollect message to queue
     * @param[in] que collect queue
     */
    virtual void collect(QueueInterface::ptr que) override;

    /**
     * @brief handle request result
     * @param[in] result req result
     */
    virtual void handler(ReqResult::ptr result) override;

protected:
    T info_;
};


// TODO create two class here, but expected one
class HardModule : Module<define::HardwareInfo> {
public:
    /**
     * @brief save config message to file
     * @param[in] filename save to file
     */
    virtual void load_from_file(const std::string & filename) override;

    /**
     * @brief indicate where module need update
     */
    virtual bool need_update() override;

    /**
     * @brief ollect message to queue
     * @param[in] que collect queue
     */
    virtual void collect(QueueInterface::ptr que) override ;

    /**
     * @brief handle request result
     * @param[in] result req result
     */
    virtual void handler(ReqResult::ptr result) override;


};


}

#endif