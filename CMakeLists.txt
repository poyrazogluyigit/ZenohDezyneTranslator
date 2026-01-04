cmake_minimum_required(VERSION 3.14)

project(ZenohDezyneTranslator)

# Clangd Setup
# This command tells CMake to generate the
# `compile_commands.json` file that clangd needs to understand your project's
# build flags, include paths, and preprocessor definitions.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# REQUIRED PACKAGES
# -----------------------------------------------------------------

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

FetchContent_Declare(
    cli11_proj
    QUIET
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
    GIT_TAG v2.6.1
)
FetchContent_MakeAvailable(cli11_proj)

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

# PARSER
# ----------------------------------------------------------------------

set(PARSER_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/parser")
set(PARSER_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/parser")

set(LEXER_OUT "${PARSER_BUILD_DIR}/lexer.cpp")
set(PARSER_OUT "${PARSER_BUILD_DIR}/parser.cpp")

bison_target(PARSER "${PARSER_SRC_DIR}/parser.y" "${PARSER_OUT}" DEFINES_FILE "${PARSER_BUILD_DIR}/parser.hpp")
flex_target(LEXER "${PARSER_SRC_DIR}/lexer.l" "${LEXER_OUT}" DEFINES_FILE "${PARSER_BUILD_DIR}/lexer.hpp")
add_flex_bison_dependency(LEXER PARSER)

add_executable(translator "${LEXER_OUT}" "${PARSER_OUT}")
target_include_directories(translator PRIVATE "${PARSER_SRC_DIR}/")


# TESTS (old)
# ----------------------------------------------------------------------

find_package(Catch2 3 REQUIRED)

add_executable(PreprocessingTests tests/PreprocessingTests.cpp)
target_include_directories(PreprocessingTests PRIVATE src/include)
target_link_libraries(PreprocessingTests PRIVATE foonathan::lexy)
target_link_libraries(PreprocessingTests PRIVATE Catch2::Catch2WithMain)

add_executable(RegistryTests tests/RegistryTests.cpp)
target_include_directories(RegistryTests PRIVATE src/include)
target_link_libraries(RegistryTests PRIVATE Catch2::Catch2WithMain)

# ---------------------------------------------------------------------


# Examples 
# ---------------------------------------------------------------------
add_executable(node1 examples/node1.cpp)
target_include_directories(node1 PRIVATE src/include)
target_link_libraries(node1 PUBLIC zenohcxx::zenohc)

add_executable(node2 examples/node2.cpp)
target_include_directories(node2 PRIVATE src/include)
target_link_libraries(node2 PUBLIC zenohcxx::zenohc)

add_custom_target(NodeExamples DEPENDS node1 node2)

add_executable(AutoLights_Controller examples/AutomaticLightsExample/Controller.cpp)
target_include_directories(AutoLights_Controller PRIVATE src/include)
target_link_libraries(AutoLights_Controller PUBLIC zenohcxx::zenohc)


add_executable(AutoLights_AutoLights examples/AutomaticLightsExample/AutoLights.cpp)
target_include_directories(AutoLights_AutoLights PRIVATE src/include)
target_link_libraries(AutoLights_AutoLights PUBLIC zenohcxx::zenohc)

add_executable(AutoLights_Timer examples/AutomaticLightsExample/Timer.cpp)
target_include_directories(AutoLights_Timer PRIVATE src/include)
target_link_libraries(AutoLights_Timer PUBLIC zenohcxx::zenohc)

add_executable(AutoLights_FrontCamera examples/AutomaticLightsExample/FrontCamera.cpp)
target_include_directories(AutoLights_FrontCamera PRIVATE src/include)
target_link_libraries(AutoLights_FrontCamera PUBLIC zenohcxx::zenohc)

add_executable(AutoLights_HighBeams examples/AutomaticLightsExample/HighBeams.cpp)
target_include_directories(AutoLights_HighBeams PRIVATE src/include)
target_link_libraries(AutoLights_HighBeams PUBLIC zenohcxx::zenohc)

add_executable(AutoLights_LightSensor examples/AutomaticLightsExample/LightSensor.cpp)
target_include_directories(AutoLights_LightSensor PRIVATE src/include)
target_link_libraries(AutoLights_LightSensor PUBLIC zenohcxx::zenohc)

add_custom_target(AutoLightsExample
    DEPENDS
        AutoLights_Controller
        AutoLights_AutoLights
        AutoLights_Timer
        AutoLights_FrontCamera
        AutoLights_HighBeams
        AutoLights_LightSensor
)

