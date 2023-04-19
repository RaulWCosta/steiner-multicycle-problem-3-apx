# Install script for directory: C:/Users/raulcosta/Documents/projetos/steiner-multicycle-problem-3-apx/libs/lemon-main-a278d16bd2d0

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/LEMON")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/cmake" TYPE FILE FILES "C:/Users/raulcosta/Documents/projetos/steiner-multicycle-problem-3-apx/libs/lemon-main-a278d16bd2d0/build/cmake/LEMONConfig.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("C:/Users/raulcosta/Documents/projetos/steiner-multicycle-problem-3-apx/libs/lemon-main-a278d16bd2d0/build/lemon/cmake_install.cmake")
  include("C:/Users/raulcosta/Documents/projetos/steiner-multicycle-problem-3-apx/libs/lemon-main-a278d16bd2d0/build/contrib/cmake_install.cmake")
  include("C:/Users/raulcosta/Documents/projetos/steiner-multicycle-problem-3-apx/libs/lemon-main-a278d16bd2d0/build/demo/cmake_install.cmake")
  include("C:/Users/raulcosta/Documents/projetos/steiner-multicycle-problem-3-apx/libs/lemon-main-a278d16bd2d0/build/tools/cmake_install.cmake")
  include("C:/Users/raulcosta/Documents/projetos/steiner-multicycle-problem-3-apx/libs/lemon-main-a278d16bd2d0/build/doc/cmake_install.cmake")
  include("C:/Users/raulcosta/Documents/projetos/steiner-multicycle-problem-3-apx/libs/lemon-main-a278d16bd2d0/build/test/cmake_install.cmake")

endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "C:/Users/raulcosta/Documents/projetos/steiner-multicycle-problem-3-apx/libs/lemon-main-a278d16bd2d0/build/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
