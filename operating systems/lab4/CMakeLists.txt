cmake_minimum_required(VERSION 3.15)
project(Lab4_Sync LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (WIN32)
    add_definitions(-DWIN32_LEAN_AND_MEAN -DNOMINMAX)
endif()

find_package(Threads REQUIRED)
enable_testing()

# GoogleTest
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
add_subdirectory(googletest)

# Основной исполняемый файл (Receiver)
add_executable(Lab4_Sync
    main.cpp
    marker_thread.cpp
    marker_thread.h
)
target_link_libraries(Lab4_Sync
    PRIVATE Threads::Threads
)

# Исполняемый файл Sender
add_executable(sender
    sender.cpp
    marker_thread.cpp
    marker_thread.h
)
target_link_libraries(sender
    PRIVATE Threads::Threads
)

# Тестовый исполняемый файл
add_executable(Lab4_Sync_Tests
    tests.cpp
    marker_thread.cpp
    marker_thread.h
)
target_compile_definitions(Lab4_Sync_Tests
    PRIVATE TESTING
)
target_link_libraries(Lab4_Sync_Tests
    PRIVATE
        GTest::gtest_main
        Threads::Threads
)

include(GoogleTest)
gtest_discover_tests(Lab4_Sync_Tests)