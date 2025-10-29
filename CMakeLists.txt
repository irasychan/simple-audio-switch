cmake_minimum_required(VERSION 3.20)
project(SimpleAudioSwitch LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

add_executable(SimpleAudioSwitch
        src/main.cpp
        src/Device.h
        src/WinError.h
        src/PolicyConfig.h
        src/AudioSwitch.cpp
        src/AudioSwitch.h
)

if (MSVC)
    add_definitions(-DUNICODE -D_UNICODE)
endif ()

target_link_libraries(SimpleAudioSwitch PRIVATE ole32 uuid)
