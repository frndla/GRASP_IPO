cmake_minimum_required(VERSION 3.5)

project(Sketch LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Sketch)
target_sources(Sketch PRIVATE 
               dataset.h
               main.cpp)
