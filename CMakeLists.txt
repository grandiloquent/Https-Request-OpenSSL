cmake_minimum_required(VERSION 3.14)
project(Http)

set(CMAKE_CXX_STANDARD 11)
add_library(Request SHARED Request.cpp)

if (WIN32)
    target_link_libraries(Request ws2_32)
endif ()

add_executable(Http main.cpp)

if (WIN32)
    target_link_libraries(Http Request)
else ()
    target_link_libraries(Http Request)
endif ()