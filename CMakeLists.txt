cmake_minimum_required(VERSION 3.14)
project(biral)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(td)

find_package(Threads REQUIRED)

add_executable(biral
    src/main.cpp
    src/profile.cpp
    src/config.cpp
)

target_link_libraries(biral PRIVATE
    Td::TdJson
    Td::TdStatic
    Threads::Threads
)

target_compile_options(biral PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

target_include_directories(biral PRIVATE
    tdlib
)