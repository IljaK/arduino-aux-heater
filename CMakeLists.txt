cmake_minimum_required(VERSION 3.10.0)
project(aux-heater-test VERSION 0.1.0)
set (CMAKE_CXX_STANDARD 11)

include(ExternalProject)

if (WIN32)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
endif (WIN32)

if (UNIX OR APPLE)
    find_package(Threads)
endif ()

# Download and install GoogleTest
ExternalProject_Add(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    INSTALL_COMMAND ""
    UPDATE_COMMAND ""
)
ExternalProject_Get_Property(googletest source_dir binary_dir)

add_library(gtestd STATIC IMPORTED)
add_library(gtest_maind STATIC IMPORTED)

if (WIN32)
    set_property(TARGET gtestd PROPERTY IMPORTED_LOCATION ${binary_dir}/lib/Debug/gtestd.lib)
    set_property(TARGET gtest_maind PROPERTY IMPORTED_LOCATION ${binary_dir}/lib/Debug/gtest_maind.lib)
endif (WIN32)

if (UNIX OR APPLE)
    set_property(TARGET gtestd PROPERTY IMPORTED_LOCATION ${binary_dir}/lib/libgtest.a)
    set_property(TARGET gtest_maind PROPERTY IMPORTED_LOCATION ${binary_dir}/lib/libgtest_main.a)
endif ()

add_dependencies(gtestd googletest)
add_dependencies(gtest_maind googletest)

include_directories(${source_dir}/googletest/include)
include_directories(arduino-stub)
include_directories(aux-heater/src/src)
include_directories(aux-heater/src/src/libs)
include_directories(aux-heater/src/src/common)
include_directories(aux-heater/src/src/serial)
include_directories(aux-heater-test/src)
include_directories(aux-heater-test/src/mock)

# Add recursive .h and .cpp files to build
file(GLOB_RECURSE aux-heater-test-src 
    "arduino-stub/*.h" "arduino-stub/*.cpp"
    "aux-heater/src/src/*.h" "aux-heater/src/src/*.cpp"
    "aux-heater-test/src/*.h" "aux-heater-test/src/*.cpp")

add_executable(aux-heater-test ${aux-heater-test-src})
target_compile_definitions(aux-heater-test PUBLIC ARDUINO_TEST)
if (UNIX OR APPLE)
    target_compile_definitions(aux-heater-test PUBLIC _LINUX_)
    target_link_libraries(aux-heater-test ${CMAKE_THREAD_LIBS_INIT})
endif ()

# Disable string safe functions warning
if (WIN32)
    target_compile_definitions(aux-heater-test PUBLIC _CRT_SECURE_NO_DEPRECATE)
    target_compile_definitions(aux-heater-test PUBLIC _CRT_NONSTDC_NO_DEPRECATE)
endif (WIN32)

target_link_libraries(aux-heater-test gtestd gtest_maind)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
