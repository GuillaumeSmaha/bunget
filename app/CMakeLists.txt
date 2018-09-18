##
##    Copyright: O-Marius Chincisan & Zirexix Inc 2016-2017
##
##    This program is distributed
##    WITHOUT ANY WARRANTY; without even the implied warranty of
##    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
##
cmake_minimum_required (VERSION 2.6)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG -Wall -ggdb")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fexceptions")

include_directories(include)
include_directories(../lib)
include_directories(../lib/include)
link_directories(../link)

set(EXECUTABLE_OUTPUT_PATH ../bin)

file(GLOB SRC_FILES *.cpp *.h)
add_executable(bunget_bin ${SRC_FILES})
target_link_libraries(bunget_bin bunget crypto++)
set_target_properties(bunget_bin PROPERTIES OUTPUT_NAME bunget)

