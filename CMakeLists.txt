cmake_minimum_required(VERSION 3.5)

project(ZenohDezyneWrapper)

# Clangd Setup
# This command tells CMake to generate the
# `compile_commands.json` file that clangd needs to understand your project's
# build flags, include paths, and preprocessor definitions.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required libraries

find_package(zenohc REQUIRED)
message(STATUS "Using ZenohC ${zenohc_VERSION}")    

find_package(zenohcxx REQUIRED)
message(STATUS "Using ZenohC++ ${zenohcxx_VERSION}")  

# Target Definitions

add_library(ZenohDezyneWrapper src/Wrapper.h src/Wrapper.cpp)

# Target Configuration

target_link_libraries(ZenohDezyneWrapper
    PUBLIC 
        zenohcxx::zenohc
    )




