cmake_minimum_required(VERSION 3.5)

project(netpp)

find_package(fmt REQUIRED)
include_directories(${fmt_INCLUDE_DIRS})

# Find the spdlog library
find_package(spdlog REQUIRED)
include_directories(${spdlog_INCLUDE_DIRS})

# Find the Boost.Asio library
find_package(Boost REQUIRED COMPONENTS system thread)

include_directories(${Boost_INCLUDE_DIRS})
set(Boost_USE_STATIC_LIBS ON)

set(Boost_USE_MULTITHREADED ON)
find_package(Boost REQUIRED COMPONENTS system thread)


add_executable(${PROJECT_NAME} NetProtocol++.cpp)


target_link_libraries(${PROJECT_NAME}
    PRIVATE fmt::fmt
    PRIVATE spdlog::spdlog
    PRIVATE Boost::system
    PRIVATE Boost::thread
)
