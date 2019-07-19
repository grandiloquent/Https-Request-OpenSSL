cmake_minimum_required(VERSION 3.14)

project(Http)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_VERBOSE_MAKEFILE ON)

find_package(OpenSSL REQUIRED)
# include_directories(${OPENSSL_INCLUDE_DIR})
if (OPENSSL_FOUND)

    include_directories(${OPENSSL_INCLUDE_DIRS})
    message(STATUS "OPENSSL_INCLUDE_DIR = ${OPENSSL_INCLUDE_DIR}")
    message(STATUS "OPENSSL_LIBRARIES = ${OPENSSL_LIBRARIES}")

endif ()


add_library(Request SHARED Request.cpp)
target_compile_definitions(Request PRIVATE "CPPHTTPLIB_OPENSSL_SUPPORT" "DEBUG")
if (WIN32)
    target_link_libraries(Request ws2_32 ssl crypto)
else ()
    target_link_libraries(Request ssl crypto)
endif ()


add_executable(Http main.cpp)

if (WIN32)
    target_link_libraries(Http Request)
else ()
    target_link_libraries(Http Request)
endif ()