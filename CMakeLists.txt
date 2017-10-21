cmake_minimum_required(VERSION 3.2)
project(PromiseC11)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--no-as-needed")


set(SOURCE_FILES
        main.cpp
        AsyncDispatcher.cpp
        SimpleDispatcher.cpp )

set(HEADER_FILES
        Include/promise.h
        AsyncDispatcher.h
        Include/PromiseResolver.h
        Include/ThreadDispatcher.h
        SimpleDispatcher.h
        )

include_directories(
        Include
)

set(
        LIBS
        pthread
)

add_executable(PromiseC11 ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(PromiseC11 ${LIBS})
