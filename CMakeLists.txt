cmake_minimum_required(VERSION 3.7)

set(CMAKE_CXX_STANDARD 17) 

project(RSDKv5SteamAPI)
set(MOD_NAME RSDKv5SteamAPI CACHE STRING "The mod directory to look into")

if (APPLE)
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()

set(OUTPUT_NAME "RSDKv5SteamAPI" CACHE STRING "The name of the built library")

if (WIN32)
    set(WIN_SRC dllmain.cpp)
endif()

add_library(RSDKv5SteamAPI SHARED
    ../RSDKv5-GameAPI/C/GameAPI/Game.c
    Mod.cpp
    Steam.cpp
    ${WIN_SRC}
)

target_include_directories(RSDKv5SteamAPI PRIVATE
    include
    include/steam
    ../RSDKv5-GameAPI/C/GameAPI
)

target_link_directories(RSDKv5SteamAPI PRIVATE
    lib/x64
    lib/x86
    lib/universal
)

target_link_libraries(RSDKv5SteamAPI PRIVATE
    steam_api
    dobby
)

set_target_properties(RSDKv5SteamAPI PROPERTIES OUTPUT_NAME ${OUTPUT_NAME})

target_compile_definitions(RSDKv5SteamAPI PRIVATE
    RETRO_REVISION=2
)

if(ANDROID)
    set_target_properties(RSDKv5SteamAPI PROPERTIES PREFIX "lib")
else()
    set_target_properties(RSDKv5SteamAPI PROPERTIES PREFIX "")
endif()
    
unset(MOD_NAME CACHE)
unset(OUTPUT_NAME CACHE)
