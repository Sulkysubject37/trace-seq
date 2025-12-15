cmake_minimum_required(VERSION 3.10)
project(traceseq CXX)
link_directories(/opt/homebrew/lib)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find libraries
find_package(yaml-cpp REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(fmt REQUIRED)
find_package(GTest REQUIRED)
find_package(cxxopts REQUIRED)

# Add executable
add_executable(traceseq cli.cpp tracer.cpp lineage.cpp hashing.cpp)

# Add include directory
target_include_directories(traceseq PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Link libraries
target_link_libraries(traceseq
    PRIVATE
    yaml-cpp
    nlohmann_json::nlohmann_json
    OpenSSL::SSL
    OpenSSL::Crypto
    fmt::fmt
    cxxopts::cxxopts
)

# Add tests
enable_testing()

add_executable(tests tests/test_runner.cpp hashing.cpp tracer.cpp lineage.cpp)
target_include_directories(tests PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(tests
    PRIVATE
    GTest::GTest
    GTest::Main
    yaml-cpp
    nlohmann_json::nlohmann_json
    OpenSSL::SSL
    OpenSSL::Crypto
    fmt::fmt
)

add_test(NAME unit_tests COMMAND tests)

# Python bindings
find_package(pybind11 REQUIRED)
find_package(nlohmann_json REQUIRED)

pybind11_add_module(traceseq_py bindings.cpp tracer.cpp lineage.cpp hashing.cpp)

target_link_libraries(traceseq_py
    PRIVATE
    yaml-cpp
    nlohmann_json::nlohmann_json
    OpenSSL::SSL
    OpenSSL::Crypto
    fmt::fmt
    ${UUID_LIBRARIES}
)
