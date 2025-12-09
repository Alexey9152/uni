cmake_minimum_required(VERSION 3.10)
project(lab5_named_pipes LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Включаем директории с заголовками
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/external
)

# Сервер с именованным каналом (WinAPI)
add_executable(server
    server.cpp
    common.h
    lock_manager.h
)

# Клиент
add_executable(client
    client.cpp
    common.h
)

# Тесты с doctest
add_executable(tests_lock_manager
    tests_lock_manager.cpp
    lock_manager.h
)

# Для тестов можно сразу включить CTest
include(CTest)
enable_testing()
add_test(NAME LockManagerTests COMMAND tests_lock_manager)
