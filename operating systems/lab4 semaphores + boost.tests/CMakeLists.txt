cmake_minimum_required(VERSION 3.15)
project(Lab4_Sync LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (WIN32)
    add_definitions(-DWIN32_LEAN_AND_MEAN -DNOMINMAX)
endif()

find_package(Boost REQUIRED COMPONENTS unit_test_framework)

enable_testing()

add_executable(Lab4_Sync
    main.cpp
    marker_thread.cpp
    marker_thread.h
)

add_executable(sender
    sender.cpp
    marker_thread.cpp
    marker_thread.h
)

add_executable(Lab4_Sync_Tests
    tests.cpp
    marker_thread.cpp
    marker_thread.h
)

target_link_libraries(Lab4_Sync_Tests
    PRIVATE Boost::unit_test_framework
)

add_test(NAME Lab4_Sync_Tests COMMAND Lab4_Sync_Tests)
