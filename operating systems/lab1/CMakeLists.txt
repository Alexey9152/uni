cmake_minimum_required(VERSION 3.14)
project(EmployeesProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Все exe будут в build/bin/<Config>
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(src)
enable_testing()
add_subdirectory(tests)
