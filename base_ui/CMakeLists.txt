cmake_minimum_required(VERSION 3.10)
project(guppyscreen C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CYR_FONTS
    src/fonts/montserrat_cyr_14.c
    src/fonts/montserrat_cyr_16.c
    src/fonts/montserrat_cyr_18.c
    src/fonts/montserrat_cyr_20.c
    src/fonts/montserrat_cyr_24.c
    src/fonts/montserrat_cyr_28.c
)

file(GLOB_RECURSE GUPPY_SOURCES
    "src/*.cpp"
    "src/fonts/*.c"
    "assets/*.c"
    "lvgl/src/*.c"
    "lv_drivers/*.c"
    "lv_touch_calibration/*.c"
    "libhv/c/*.c"
    "libhv/cpp/*.cpp"
    "libhv/base/*.c"
    "libhv/event/*.c"
    "libhv/http/*.c"
    "libhv/http/client/*.cpp"
    "libhv/http/server/*.cpp"
)

add_executable(guppyscreen ${GUPPY_SOURCES} ${CYR_FONTS})

target_include_directories(guppyscreen PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/fonts
    ${CMAKE_CURRENT_SOURCE_DIR}/lvgl
    ${CMAKE_CURRENT_SOURCE_DIR}/lv_drivers
    ${CMAKE_CURRENT_SOURCE_DIR}/lv_touch_calibration
    ${CMAKE_CURRENT_SOURCE_DIR}/spdlog/include
    ${CMAKE_CURRENT_SOURCE_DIR}/libhv/include
    ${CMAKE_CURRENT_SOURCE_DIR}/libhv/include/hv
    ${CMAKE_CURRENT_SOURCE_DIR}/wpa_supplicant/src/common
)

target_link_libraries(guppyscreen PRIVATE
    pthread
    atomic
    m
)
