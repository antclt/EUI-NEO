cmake_minimum_required(VERSION 3.14)
project(EuiNeoFindPackageConsumer LANGUAGES CXX)

find_package(EuiNeo CONFIG REQUIRED)

add_executable(eui_find_package_consumer main.cpp)
target_link_libraries(eui_find_package_consumer PRIVATE eui::neo)
