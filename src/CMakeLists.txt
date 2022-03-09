# Copyright © 2013 Canonical Ltd.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Authored by: Thomas Voss <thomas.voss@canonical.com>

find_package(PkgConfig)
pkg_check_modules(DBUS dbus-1)

include_directories(
  ${DBUS_INCLUDE_DIRS}
  ${BOOST_INCLUDE_DIRS}
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -Wall -fno-strict-aliasing -fvisibility=hidden -fvisibility-inlines-hidden -Wextra -fPIC")

add_library(
  ${PROJECT_NAME} SHARED
  plugin.cc
  data.cc
  application.cc
  config.cc
  define.pb.cc
  log.cc
  utils.cc
  writer.cc
  resp_chain.cc
  define.cc
)

target_link_libraries (
  ${PROJECT_NAME}

  ${BOOST_LIBRARIES}
  ${DBUS_CPP_LIBRARIES}
  ${CRYPTOPP_LIBRARIES}
  ${CPR_LIBRARIES}
  ${SQLITE3_LIBRARIES}
  ${PROTOBUF_LIBRARIES}
)

install(TARGET ${PLUGIN_NAME} LIBRARY DESTINATION lib/dde-plugins/session)