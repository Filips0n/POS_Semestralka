cmake_minimum_required(VERSION 3.7)
project(POS_Semestralka)

set(CMAKE_CXX_STANDARD 14)

add_executable(server server/server.cpp)
add_executable(klient klient/klient.cpp)

target_link_libraries(server pthread)
target_link_libraries(klient pthread)