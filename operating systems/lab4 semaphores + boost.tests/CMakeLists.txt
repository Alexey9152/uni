cmake_minimum_required(VERSION 3.15)
project(Lab4_Sync_Boost LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (WIN32)
    add_definitions(-DWIN32_LEAN_AND_MEAN -DNOMINMAX)
endif()

# Ищем оба пакета через vcpkg
set(CMAKE_PREFIX_PATH "C:/Users/Alexey/vcpkg/installed/x64-windows")

find_package(Boost REQUIRED COMPONENTS unit_test_framework)

enable_testing()

# Основное приложение Receiver
add_executable(Lab4_Sync
    main.cpp
    marker_thread.cpp
    marker_thread.h
)


# Sender
add_executable(sender
    sender.cpp
    marker_thread.cpp
    marker_thread.h
)


# Тесты
add_executable(Lab4_Sync_Tests
    tests.cpp
    marker_thread.cpp
    marker_thread.h
)


add_test(NAME Lab4_Sync_Tests COMMAND Lab4_Sync_Tests)
