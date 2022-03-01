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
#include "interface.h"
#include <memory>

namespace experience {

// HardwareModule use to update and upost hardware info
class HardwareModule : public ModuleCfgCor {
public:
    typedef std::shared_ptr<HardwareModule> ptr;

    /**
     * @brief Construct a new Hardware Module object
     */
    HardwareModule();

    /**
     * @brief Destroy the virtual Hardware Module object
     */
    virtual~HardwareModule();

    /**
     * @brief save config message to file
     * @param filename save to file
     */
    virtual void save_to_file(const std::string & filename = hardware_file) override;

    /**
     * @brief load config from file
     * @param[in] filename file name
     */
    virtual void load_from_file(const std::string & filename = hardware_file) override;

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
};





}

#endif