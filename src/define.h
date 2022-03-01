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


#include <memory>
#include <string>
namespace experience {

const std::string experience_dir = "/var/public/deepin-user-experience";
const std::string hardware_file = experience_dir + "/" + "hardware";

struct ReqMessage {
typedef std::shared_ptr<ReqMessage> ptr;
};


struct ReqResult {
typedef std::shared_ptr<ReqResult> ptr;
};

}

#endif