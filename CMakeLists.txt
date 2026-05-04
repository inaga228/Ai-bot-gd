cmake_minimum_required(VERSION 3.21)
project(AILevelBot VERSION 1.0.0)

add_library(${PROJECT_NAME}
    src/main.cpp
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

if (DEFINED ENV{GEODE_SDK} AND EXISTS "$ENV{GEODE_SDK}/CMakeLists.txt")
    message(STATUS "Using GEODE_SDK at $ENV{GEODE_SDK}")
    add_subdirectory("$ENV{GEODE_SDK}" "${CMAKE_CURRENT_BINARY_DIR}/geode-sdk")
else()
    message(FATAL_ERROR
        "Geode SDK was not found.\n"
        "Set GEODE_SDK to a Geode SDK checkout or run Geode CLI SDK install first.\n"
        "Docs: https://docs.geode-sdk.org/getting-started/geode-cli"
    )
endif()

setup_geode_mod(${PROJECT_NAME})
