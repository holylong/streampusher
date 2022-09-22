# MIT License

# Copyright (c) 2022 hooy

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.


cmake_minimum_required(VERSION 3.11)


project(StreamPusher VERSION 0.1 LANGUAGES CXX C)

set(STREAMPUSHER_DIR ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB StreamPusher_SRCS ${STREAMPUSHER_DIR}/*.c ${STREAMPUSHER_DIR}/*.cpp ${STREAMPUSHER_DIR}/*.h)

message(STATUS "==>> ${StreamPusher_SRCS}")

add_definitions(-DRTMPPUSHER_EXPORTS)

if(OpenCV_FOUND)
include_directories(${OpenCV_INCLUDE_DIRS})
endif()

include_directories(${STREAMPUSHER_DIR} ${FFMPEG_INCLUDE_DIRS})
add_library(StreamPusher SHARED ${StreamPusher_SRCS})
target_link_libraries(StreamPusher PRIVATE ${FFMPEG_LIBRARIES})

if(OpenCV_FOUND)
target_link_libraries(StreamPusher PRIVATE ${OpenCV_LIBRARIES})
endif()

set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})