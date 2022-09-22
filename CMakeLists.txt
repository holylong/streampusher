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

project(Pusher VERSION 0.1 LANGUAGES CXX C)

set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

add_definitions(-DUSE_CMAKE_BUILD -D_CRT_SECURE_NO_WARNINGS)

find_package(OpenCV REQUIRED)
find_package(ffmpeg QUIET REQUIRED)

if(OpenCV_FOUND)
add_definitions(-DUSE_OPENCV)
endif()

add_subdirectory(${ROOT_DIR}/DXGICapture)
add_subdirectory(${ROOT_DIR}/StreamPusher)

file(GLOB_RECURSE Pusher_SRCS RELATIVE ${ROOT_DIR} ${ROOT_DIR}/Pusher/*.cpp)

message(STATUS "==>> ${Pusher_SRCS}")
list(REMOVE_ITEM Pusher_SRCS "Pusher/FilePusher.cpp")

include_directories(${ROOT_DIR}/StreamPusher ${ROOT_DIR}/DXGICapture ${OpenCV_INCLUDE_DIRS})

if(OpenCV_FOUND)
add_executable(CapturePusher ${Pusher_SRCS})
target_link_libraries(CapturePusher PRIVATE ${OpenCV_LIBRARIES} StreamPusher)
target_link_libraries(CapturePusher PRIVATE DXGICapture)
add_executable(FilePusher ${ROOT_DIR}/Pusher/FilePusher.cpp)
target_link_libraries(FilePusher PRIVATE ${OpenCV_LIBRARIES} StreamPusher)
endif()


set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})