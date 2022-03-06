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
find_package(Boost COMPONENTS system)
find_package(Threads)

pkg_check_modules(DBUS dbus-1)

DBUS_CFLAGS:INTERNAL=-I/usr/include/dbus-1.0;-I/usr/lib/dbus-1.0/include
DBUS_INCLUDE_DIRS:INTERNAL=/usr/include/dbus-1.0;/usr/lib/dbus-1.0/include
DBUS_LIBRARIES:INTERNAL=dbus-1
DBUS_CPP_LIBRARIES:INTERNAL=dbus-cpp
PROTOBUF_LIBRARIES:INTERNAL=protobuf
CPR_LIBRARIES:INTERNAL=cpr
CRPYPTO_LIBRARIES:INTERNAL=cryptopp
SQLITE3_LIBRARIES:INTERNAL=sqlite3


// flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Werror -Wall -Wextra -fvisibility=hidden -fPIC -pthread")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -Werror -Wall -fno-strict-aliasing -fvisibility=hidden -fvisibility-inlines-hidden -Wextra -fPIC -pthread")


include_directories(
  ${CMAKE_SOURCE_DIR}/include
  ${DBUS_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  ${GTEST_INCLUDE_DIRS}
  )

add_library(
  DSService SHARED

  ${CMAKE_CURRENT_BINARY_DIR}/data.cc
  application.cc
  config.cc
  define.pb.cc
  log.cc
  utils.cc
  writer.cc
)

target_link_libraries(
  DSService

  ${Boost_LIBRARIES}
  ${DBUS_LIBRARIES}
  ${DBUS_CPP_LIBRARIES}
  ${PROTOBUF_LIBRARIES}
  ${CPR_LIBRARIES}

) 