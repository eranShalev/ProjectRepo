cmake_minimum_required(VERSION 3.5)
project(main)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

include_directories(/usr/local/include/libnetfilter_queue)

set(SOURCE_FILES main.cpp)

add_executable(main ${SOURCE_FILES})

set(NETFILTER_LIBRARY /usr/local/lib)

target_link_libraries(main PRIVATE "${NETFILTER_LIBRARY}/libnetfilter_queue.so" "${NETFILTER_LIBRARY}/libnfnetlink.so")

#from here starts google testing lines
enable_testing()

add_subdirectory(Tests)
