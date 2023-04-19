# CMakeList.txt : CMake project for steiner-multicycle-problem-3-apx, include source and define
# project specific logic here.

cmake_minimum_required(VERSION 3.8)

set(PROJECT_NAME "steiner-multicycle-problem-3-apx")

project(${PROJECT_NAME})

option(CXX "enable C++ compilation" ON)

if(CXX)
  enable_language(CXX)
endif()

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Visual Studio compiler with static runtime libraries
if(MSVC AND MT)
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MT")
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MTd")
endif()

find_package(GUROBI REQUIRED)

include_directories(${GUROBI_INCLUDE_DIRS})




# include_directories(libs/lemon-main-a278d16bd2d0 libs/lemon-main-a278d16bd2d0/build)

# list source files here
set(sources main.cpp libs/lemon-main-a278d16bd2d0/lemon/bits/lwindows.cc)

add_executable(${CMAKE_PROJECT_NAME} ${sources})

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libs/lemon-main-a278d16bd2d0/)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libs/lemon-main-a278d16bd2d0/build/)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libs/lemon-main-a278d16bd2d0/lemon/bits/)

if(CXX)
  set(CMAKE_CXX_STANDARD 11)
  target_link_libraries(${CMAKE_PROJECT_NAME} optimized ${GUROBI_CXX_LIBRARY}
                        debug ${GUROBI_CXX_DEBUG_LIBRARY})
endif()


# add_library(LEMON MODULE libs/lemon-main-a278d16bd2d0/lemon/)

# set_target_properties(LEMON PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries(${CMAKE_PROJECT_NAME} ${GUROBI_LIBRARY})

# target_link_libraries(${CMAKE_PROJECT_NAME} LEMON)

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --without-glpk --without-cplex --without-soplex")


if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  include(FeatureSummary)
  feature_summary(WHAT ALL)
endif()
