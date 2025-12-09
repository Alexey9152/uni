cmake_minimum_required(VERSION 3.16)
project(NamedPipeLab LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Включаем поддержку тестов
enable_testing()  # [web:511]

# Пути к исходникам (если надо)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Ищем Boost: thread, system (для потоков) + unit_test_framework (для тестов) [web:502][web:509]
find_package(Boost REQUIRED COMPONENTS thread system unit_test_framework)

# Сервер
add_executable(Server
    Server.cpp
    protocol.h
)
target_link_libraries(Server PRIVATE Boost::thread Boost::system)

# Клиент
add_executable(Client
    Client.cpp
    protocol.h
)
target_link_libraries(Client PRIVATE Boost::thread Boost::system)

# ---------- Тесты на Boost.Test ----------

# Исполняемый файл с тестами
add_executable(LockManagerTests
    tests/LockManagerTests.cpp
)

target_link_libraries(LockManagerTests
    PRIVATE Boost::unit_test_framework Boost::thread Boost::system
)

# Регистрируем тест для ctest
add_test(NAME LockManagerTests COMMAND LockManagerTests)
