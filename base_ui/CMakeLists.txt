cmake_minimum_required(VERSION 3.10)
project(guppyscreen C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE GUPPY_SOURCES
    "src/*.cpp"
    "assets/*.c"
    "lvgl/src/*.c"
    "lv_drivers/*.c"
    "lv_touch_calibration/*.c"
)

add_executable(guppyscreen ${GUPPY_SOURCES})

target_include_directories(guppyscreen PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/lvgl
    ${CMAKE_CURRENT_SOURCE_DIR}/lv_drivers
    ${CMAKE_CURRENT_SOURCE_DIR}/lv_touch_calibration
    ${CMAKE_CURRENT_SOURCE_DIR}/spdlog/include
    ${CMAKE_CURRENT_SOURCE_DIR}/libhv/include
    ${CMAKE_CURRENT_SOURCE_DIR}/wpa_supplicant/src/common
)

target_link_libraries(guppyscreen PRIVATE
    pthread
    atomic
    m
)
