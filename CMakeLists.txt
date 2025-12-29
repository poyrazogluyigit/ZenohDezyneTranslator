cmake_minimum_required(VERSION 3.8)

project(ZenohDezyneWrapper)

# Clangd Setup
# This command tells CMake to generate the
# `compile_commands.json` file that clangd needs to understand your project's
# build flags, include paths, and preprocessor definitions.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required libraries

include(FetchContent)

find_package(zenohc REQUIRED)
message(STATUS "Using ZenohC ${zenohc_VERSION}")    

find_package(zenohcxx REQUIRED)
message(STATUS "Using ZenohC++ ${zenohcxx_VERSION}")  

FetchContent_Declare(
    lexy
    GIT_REPOSITORY https://github.com/foonathan/lexy.git
    GIT_TAG        v2025.05.0  
)
FetchContent_MakeAvailable(lexy)

find_package(Catch2 3 REQUIRED)

add_executable(PreprocessingTests tests/PreprocessingTests.cpp)
target_include_directories(PreprocessingTests PRIVATE src/include)
target_link_libraries(PreprocessingTests PRIVATE foonathan::lexy)
target_link_libraries(PreprocessingTests PRIVATE Catch2::Catch2WithMain)

add_executable(RegistryTests tests/RegistryTests.cpp)
target_include_directories(RegistryTests PRIVATE src/include)
target_link_libraries(RegistryTests PRIVATE Catch2::Catch2WithMain)

# Target Definitions


# add_library(ZenohDezyneWrapper src/Wrapper.h src/Wrapper.cpp)
# add_executable(node1 src/node1.cpp)
# add_executable(node2 src/node2.cpp)
# # add_executable(main src/main.cpp src/Enum.h)
# 
# # Target Configuration
# 
# target_link_libraries(ZenohDezyneWrapper
#     PUBLIC 
#         zenohcxx::zenohc
#     )
# 
# target_link_libraries(node1
#     PUBLIC
#         ZenohDezyneWrapper
# )
# 
# target_link_libraries(node2
#     PUBLIC
#         ZenohDezyneWrapper
# )




