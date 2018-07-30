cmake_minimum_required(VERSION 3.0.0)
project(tarski)

include_directories(src)
file(GLOB SOURCES "src/*.cpp")
file(GLOB SOURCES "src/*/*.cpp")
file(GLOB SOURCES "src/*.c")
file(GLOB SOURCES "src/*/*.c")
add_executable(tarski ${SOURCES})
