cmake_minimum_required(VERSION 3.12)
project(uhsm_test LANGUAGES C CXX ASM)

include(FetchContent)
FetchContent_Declare(
  cpputest
  GIT_REPOSITORY https://github.com/cpputest/cpputest.git
)
FetchContent_GetProperties(cpputest)
if(NOT cpputest_POPULATED)
  message(STATUS "Populating CppUTest...")
  FetchContent_Populate(cpputest)
endif()

set(CppUTestRootDirectory ${cpputest_SOURCE_DIR})
add_subdirectory(${CppUTestRootDirectory}/src/CppUTest)
add_subdirectory(${CppUTestRootDirectory}/src/CppUTestExt)
target_sources(CppUTest PRIVATE ${CppUTestRootDirectory}/src/Platforms/Gcc/UtestPlatform.cpp)
target_sources(CppUTestExt PRIVATE ${CppUTestRootDirectory}/src/Platforms/Gcc/UtestPlatform.cpp)
target_compile_definitions(CppUTest PUBLIC CPPUTEST_MEM_LEAK_DETECTION_DISABLED)
target_compile_definitions(CppUTestExt PUBLIC CPPUTEST_MEM_LEAK_DETECTION_DISABLED)

add_executable(uhsm_test
  src/sample_test.cpp
  src/transition_test.cpp
  src/action_test.cpp
  src/main.cpp
)
target_include_directories(uhsm_test PRIVATE ../include)
target_compile_features(uhsm_test PRIVATE cxx_std_17)
target_link_libraries(uhsm_test PRIVATE CppUTest CppUTestExt)
