cmake_minimum_required (VERSION 3.5)

project (Server)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -std=c++14")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")



add_executable (Server 
Server/src/Entrypoint.cpp 
Server/src/network/Server.cpp
Server/src/network/Server.h
)


#Init pthread
find_package(Threads)
target_link_libraries(Server ${CMAKE_THREAD_LIBS_INIT})

