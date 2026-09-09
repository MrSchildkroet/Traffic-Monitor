cmake_minimum_required(VERSION 3.20)
project(traffic_monitor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(
    traffic_monitor

    src/main.cpp
    src/cnm.cpp
    src/sources/snapshot_source.cpp
    src/sources/netevent_source.cpp
    src/core/snapshot_differ.cpp
    src/output/console_writer.cpp
    src/core/scope_classifier.cpp
    src/core/domain_classifier.cpp

)

target_include_directories(traffic_monitor PRIVATE src)

target_link_libraries(
    traffic_monitor
    PRIVATE

    iphlpapi
    ws2_32
    advapi32
    fwpuclnt
)

if(MSVC)
    target_compile_options(traffic_monitor PRIVATE /W4)
    # UNICODE konsistent, damit die *W-APIs sauber auflösen
    target_compile_definitions(traffic_monitor PRIVATE UNICODE _UNICODE NTDDI_VERSION=0x0A000006 _WIN32_WINNT=0x0A00)
endif()
