cmake_minimum_required(VERSION 3.15)
project(Lab3_Sync LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (WIN32)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
endif()

find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS thread system)
find_package(Catch2 3 REQUIRED)

enable_testing()

# Основной исполняемый файл
add_executable(Lab3_Sync
    main.cpp
    marker_thread.cc
    marker_thread.h
)
target_link_libraries(Lab3_Sync
    PRIVATE 
        Threads::Threads
        Boost::thread
        Boost::system
)

# Тестовый исполняемый файл с Catch2
add_executable(Lab3_Sync_Tests
    tests_catch2.cpp
    marker_thread.cc
    marker_thread.h
)
target_link_libraries(Lab3_Sync_Tests
    PRIVATE
        Catch2::Catch2WithMain
        Threads::Threads
        Boost::thread
        Boost::system
)

# Регистрация тестов для CTest
add_test(NAME MarkerThreadTests COMMAND Lab3_Sync_Tests)
