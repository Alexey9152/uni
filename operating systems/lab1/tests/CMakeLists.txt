# tests/CMakeLists.txt

# Подключаем googletest из external/googletest
add_subdirectory(${CMAKE_SOURCE_DIR}/external/googletest
                 ${CMAKE_BINARY_DIR}/external/googletest-build)

add_executable(integration_tests test_integration.cpp)
target_link_libraries(integration_tests PRIVATE gtest_main)

target_include_directories(integration_tests PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Пусть тесты знают, где exe (bin/<Config>)
target_compile_definitions(integration_tests PRIVATE
    TEST_BIN_DIR="${CMAKE_BINARY_DIR}/bin/$<CONFIG>"
)

if(TARGET mainapp)
  target_compile_definitions(integration_tests PRIVATE HAVE_MAINAPP=1)
endif()

include(GoogleTest)
gtest_discover_tests(integration_tests)

