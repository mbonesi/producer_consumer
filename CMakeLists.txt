cmake_minimum_required(VERSION 3.0.0)
project(prodcons VERSION 0.1.0 LANGUAGES C CXX)

add_executable(prodcons main.cpp)
target_link_libraries(prodcons pthread)