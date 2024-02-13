cmake_minimum_required(VERSION 3.10)

project(Client_Server_APP VERSION 1.0.0)

find_package(Boost COMPONENTS system thread program_options REQUIRED)

add_executable(client source/client.cpp)
add_executable(server source/server.cpp)

target_link_libraries(client PRIVATE Boost::system Boost::thread Boost::program_options)
target_link_libraries(server PRIVATE Boost::system Boost::thread Boost::program_options)

set_target_properties(client server PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
)
