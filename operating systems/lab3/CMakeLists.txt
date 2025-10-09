cmake_minimum_required(VERSION 3.15)
project(Lab3_Sync LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (WIN32)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
endif()

find_package(Threads REQUIRED)
enable_testing()

# GoogleTest
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
add_subdirectory(googletest)

# Основной исполняемый файл
add_executable(Lab3_Sync
    main.cpp
    marker_thread.cpp
    marker_thread.h
)
target_link_libraries(Lab3_Sync
    PRIVATE Threads::Threads
)

# Тестовый исполняемый файл (берёт код из теста + из marker_thread.cpp)
add_executable(Lab3_Sync_Tests
    tests.cpp
    marker_thread.cpp
    marker_thread.h
)
# чтобы в main.cpp не определялся main() - но у нас main.cpp не входит в тесты
target_compile_definitions(Lab3_Sync_Tests
    PRIVATE TESTING
)
target_link_libraries(Lab3_Sync_Tests
    PRIVATE
        GTest::gtest_main
        Threads::Threads
)

include(GoogleTest)
gtest_discover_tests(Lab3_Sync_Tests)