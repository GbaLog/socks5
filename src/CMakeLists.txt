cmake_minimum_required(VERSION 3.10)

project(socks5_proxy)

add_subdirectory(ratellib)

add_executable(${PROJECT_NAME} main.cpp)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
target_link_libraries(${PROJECT_NAME} ratel-lib)
target_include_directories(${PROJECT_NAME} PRIVATE ratellib)
