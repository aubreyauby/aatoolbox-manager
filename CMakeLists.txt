cmake_minimum_required(VERSION 3.16)
project(aaToolboxManager)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

file(GLOB SOURCES "src/*.cpp")

add_executable(aaToolboxManager WIN32 ${SOURCES})

target_link_libraries(aaToolboxManager PRIVATE user32)
