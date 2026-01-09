cmake_minimum_required(VERSION 3.20)
project (nirvana
    VERSION 1.0
    LANGUAGES CXX
)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wpedantic)
include_directories(include)
add_executable(nirvana
    src/main.cpp
    src/server.cpp
    src/Router.cpp
    src/HttpResponse.cpp
    src/Songs.cpp
    src/utils.cpp
)
