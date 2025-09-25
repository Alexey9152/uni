cmake_minimum_required(VERSION 3.15)
project(Lab2_Threads LANGUAGES CXX)

set(CMAKE_CXX_STANDARD     17)
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
add_executable(Lab2_Threads
    main.cpp
)
target_link_libraries(Lab2_Threads
    PRIVATE Threads::Threads
)

# Тестовый исполняемый файл (берёт код из теста + из main.cpp)
add_executable(Lab2_Threads_Tests
    test.cpp
    main.cpp
)
# чтобы в main.cpp не определялся main()
target_compile_definitions(Lab2_Threads_Tests
    PRIVATE TESTING
)
target_link_libraries(Lab2_Threads_Tests
    PRIVATE
        GTest::gtest_main
        Threads::Threads
)

include(GoogleTest)
gtest_discover_tests(Lab2_Threads_Tests)
