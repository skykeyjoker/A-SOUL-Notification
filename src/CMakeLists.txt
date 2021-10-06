cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENTDIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

project ("ASoulNotification")

find_package(Qt5 COMPONENTS Core Network)
include_directories(${CMAKE_CURRENT_LIST_DIR}/3rd)

set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/logo.rc")

add_executable (ASoulNotification ${APP_ICON_RESOURCE_WINDOWS} "ASoulNotification.cpp"  "BilibiliMessage.h" "BilibiliMessage.cpp" "Essential.h" "3rd/json.hpp" "3rd/wintoastlib.h" "3rd/wintoastlib.cpp")

target_link_libraries(ASoulNotification Qt5::Core Qt5::Network)

