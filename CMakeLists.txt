cmake_minimum_required(VERSION 3.20)
project(SimpleAudioSwitch)

set(CMAKE_CXX_STANDARD 20)

add_executable(SimpleAudioSwitch main.cpp)

target_link_libraries(SimpleAudioSwitch
        ole32
        uuid
)
