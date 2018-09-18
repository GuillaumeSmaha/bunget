##
##    Copyright: O-Marius Chincisan 2016-2017 Zirexix Inc
##
##    This program is distributed
##    WITHOUT ANY WARRANTY; without even the implied warranty of
##    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
##
##    The library or code from is restricted to be used in commercial products
##    without bunget.cert file. Please obtain a custom cert file from admin@meeiot.org
##
##
cmake_minimum_required (VERSION 2.6)

include_directories ("./include")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG -Wall -ggdb")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(LIBRARY_OUTPUT_PATH ../link)

file(GLOB SRC_FILES *.cpp *.h)
add_library(bunget SHARED ${SRC_FILES})
add_library(bunget_static STATIC ${SRC_FILES})
set_target_properties(bunget_static PROPERTIES OUTPUT_NAME bunget)
