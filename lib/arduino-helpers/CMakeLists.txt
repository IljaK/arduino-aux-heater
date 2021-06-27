# Custom function in root CmakeList.txt for include directroy dependencies
include_dir(${CMAKE_CURRENT_LIST_DIR}/src)
include_dir(${CMAKE_CURRENT_LIST_DIR}/gtest)

# Add recursive .h and .cpp files to test build
file(GLOB temp_src_code_list 
"${CMAKE_CURRENT_LIST_DIR}/src/*.h" "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp"
"${CMAKE_CURRENT_LIST_DIR}/src/array/*.h" "${CMAKE_CURRENT_LIST_DIR}/src/array/*.cpp"
"${CMAKE_CURRENT_LIST_DIR}/src/common/*.h" "${CMAKE_CURRENT_LIST_DIR}/src/common/*.cpp"
"${CMAKE_CURRENT_LIST_DIR}/src/measurements/*.h" "${CMAKE_CURRENT_LIST_DIR}/src/measurements/*.cpp"
"${CMAKE_CURRENT_LIST_DIR}/src/serial/*.h" "${CMAKE_CURRENT_LIST_DIR}/src/serial/*.cpp")

add_source("${temp_src_code_list}")

# Add recursive .h and .cpp files to test build
file(GLOB temp_src_test_list 
"${CMAKE_CURRENT_LIST_DIR}/gtest/*.h" "${CMAKE_CURRENT_LIST_DIR}/gtest/*.cpp"
"${CMAKE_CURRENT_LIST_DIR}/gtest/array-test/*.h" "${CMAKE_CURRENT_LIST_DIR}/gtest/array-test/*.cpp"
"${CMAKE_CURRENT_LIST_DIR}/gtest/mock/*.h" "${CMAKE_CURRENT_LIST_DIR}/gtest/mock/*.cpp"
"${CMAKE_CURRENT_LIST_DIR}/gtest/serial-test/*.h" "${CMAKE_CURRENT_LIST_DIR}/gtest/serial-test/*.cpp")

add_source("${temp_src_test_list}")
