# src/CMakeLists.txt

add_library(common INTERFACE)
target_include_directories(common INTERFACE ${CMAKE_SOURCE_DIR}/include)

# Creator
add_executable(creator creator.cpp)
target_link_libraries(creator PRIVATE common)
set_target_properties(creator PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/Debug
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/Release
)

# Reporter
add_executable(reporter reporter.cpp)
target_link_libraries(reporter PRIVATE common)
set_target_properties(reporter PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/Debug
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/Release
)

# MainApp (только Windows)
if(WIN32)
  add_executable(mainapp main.cpp)
  target_link_libraries(mainapp PRIVATE common)
  set_target_properties(mainapp PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
      RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/Debug
      RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/Release
  )
else()
  message(STATUS "main.cpp uses Windows API; skipping build of mainapp on non-Windows hosts.")
endif()
