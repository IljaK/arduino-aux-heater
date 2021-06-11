cmake_minimum_required(VERSION 3.10.0)
project(aux-heater-test VERSION 0.1.0)
set (CMAKE_CXX_STANDARD 11)
set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)

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
include_directories(aux-heater/src)
include_directories(aux-heater/src/gsm)
include_directories(aux-heater-test/src)
include_directories(aux-heater-test/src/mock)
include_directories(aux-heater-test/src/array)
include_directories(aux-heater/lib/arduino-helpers/src)
include_directories(aux-heater/lib/arduino-helpers/src/array)
include_directories(aux-heater/lib/arduino-helpers/src/common)
include_directories(aux-heater/lib/arduino-helpers/src/measurements)
include_directories(aux-heater/lib/arduino-helpers/src/serial)
include_directories(aux-heater/lib/async-GSM/src)
include_directories(aux-heater/lib/async-GSM/src/common)
include_directories(aux-heater/lib/async-GSM/src/socket)

# Add recursive .h and .cpp files to build
file(GLOB aux-heater-test-src 
    "arduino-stub/*.h" "arduino-stub/*.cpp"
    "aux-heater/src/*.h" "aux-heater/src/*.cpp"
    "aux-heater/src/gsm/*.h" "aux-heater/src/gsm/*.cpp"
    "aux-heater-test/src/*.h" "aux-heater-test/src/*.cpp"
    "aux-heater-test/src/mock/*.h" "aux-heater-test/src/mock/*.cpp"
    "aux-heater-test/src/array/*.h" "aux-heater-test/src/array/*.cpp"
    "aux-heater/lib/arduino-helpers/src/*.h" "aux-heater/lib/arduino-helpers/src/*.cpp"
    "aux-heater/lib/arduino-helpers/src/array/*.h" "aux-heater/lib/arduino-helpers/src/array/*.cpp"
    "aux-heater/lib/arduino-helpers/src/common/*.h" "aux-heater/lib/arduino-helpers/src/common/*.cpp"
    "aux-heater/lib/arduino-helpers/src/serial/*.h" "aux-heater/lib/arduino-helpers/src/serial/*.cpp"
    "aux-heater/lib/arduino-helpers/src/measurements/*.h" "aux-heater/lib/arduino-helpers/src/measurements/*.cpp"
    "aux-heater/lib/async-GSM/src/*.h" "aux-heater/lib/async-GSM/src/*.cpp"
    "aux-heater/lib/async-GSM/src/common/*.h" "aux-heater/lib/async-GSM/src/common/*.cpp"
    "aux-heater/lib/async-GSM/src/socket/*.h" "aux-heater/lib/async-GSM/src/socket/*.cpp")

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
