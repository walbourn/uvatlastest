# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.20)

project (UVAtlasTest
  DESCRIPTION "UVAtlas Test Suite"
  HOMEPAGE_URL "https://github.com/walbourn/uvatlastest/wiki"
  LANGUAGES CXX)

option(BUILD_BVT "Build-verification test" OFF)

if(CMAKE_VERSION VERSION_LESS 3.21)
  get_property(not_top DIRECTORY PROPERTY PARENT_DIRECTORY)
  if(not_top)
    set(PROJECT_IS_TOP_LEVEL false)
  else()
    set(PROJECT_IS_TOP_LEVEL true)
  endif()
endif()

if(PROJECT_IS_TOP_LEVEL)
  message(FATAL_ERROR "UVAtlas Test Suite should be built by the main CMakeLists")
endif()

set(TEST_EXES headertest xtuvatlas)
add_executable(headertest headertest/main.cpp)

if(WIN32 AND (NOT DEFINED ENV{DIRECTXTEX_MEDIA_PATH}) AND (NOT BUILD_BVT))
  message(WARNING "Tests make use of DIRECTXTEX_MEDIA_PATH environment variable to find media")
endif()

if(WIN32 AND (NOT DEFINED ENV{DIRECTXMESH_MEDIA_PATH}) AND (NOT BUILD_BVT))
  message(WARNING "Tests make use of DIRECTXMESH_MEDIA_PATH environment variable to find media")
endif()

if(BUILD_BVT)
  message(STATUS "Building for Build Verification Tests (BVTs)")
  set(BVT_MEDIA_PATH "${CMAKE_CURRENT_LIST_DIR}/media")
  cmake_path(NATIVE_PATH BVT_MEDIA_PATH BVT_MEDIA_PATH)
endif()

add_executable(xtuvatlas
   atlas.cpp
   imt.cpp
   process.cpp
   utils.cpp
   directxtest.cpp)
add_test(NAME "uvatlas" COMMAND xtuvatlas)
set_tests_properties(uvatlas PROPERTIES TIMEOUT 600)
if(BUILD_BVT)
  set_tests_properties(uvatlas PROPERTIES ENVIRONMENT "DIRECTXMESH_MEDIA_PATH=${BVT_MEDIA_PATH};DIRECTXTEX_MEDIA_PATH=${BVT_MEDIA_PATH}")
endif()

message(STATUS "Enabled tests: ${TEST_EXES}")
foreach(t IN LISTS TEST_EXES)
  target_link_libraries(${t} PRIVATE UVAtlas)
endforeach()

if(directxmath_FOUND)
    foreach(t IN LISTS TEST_EXES)
      target_link_libraries(${t} PRIVATE Microsoft::DirectXMath)
    endforeach()
endif()

if(directx-headers_FOUND)
    foreach(t IN LISTS TEST_EXES)
      target_link_libraries(${t} PRIVATE Microsoft::DirectX-Headers)
      target_compile_definitions(${t} PRIVATE USING_DIRECTX_HEADERS)
    endforeach()
endif()

if(MSVC)
    foreach(t IN LISTS TEST_EXES)
      target_compile_options(${t} PRIVATE "$<IF:$<STREQUAL:${t},headertest>,/Wall,/W4>" /GR-)
    endforeach()
endif()

if(DEFINED COMPILER_DEFINES)
    foreach(t IN LISTS TEST_EXES)
      target_compile_definitions(${t} PRIVATE ${COMPILER_DEFINES})
      target_compile_options(${t} PRIVATE ${COMPILER_SWITCHES})
      target_link_options(${t} PRIVATE ${LINKER_SWITCHES})
    endforeach()
endif()

if(BUILD_BVT)
    foreach(t IN LISTS TEST_EXES)
      target_compile_definitions(${t} PRIVATE -DBUILD_BVT_ONLY)
    endforeach()
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|IntelLLVM")
    target_compile_options(headertest PRIVATE -Wall -Wpedantic -Wextra)

    set(WarningsEXE "-Wno-c++98-compat" "-Wno-c++98-compat-pedantic" "-Wno-language-extension-token")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 16.0)
        list(APPEND WarningsEXE "-Wno-unsafe-buffer-usage")
    endif()
    foreach(t IN LISTS TEST_EXES)
      target_compile_options(${t} PRIVATE ${WarningsEXE})
    endforeach()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    foreach(t IN LISTS TEST_EXES)
      target_compile_options(${t} PRIVATE "-Wno-ignored-attributes" "-Walloc-size-larger-than=4GB")
    endforeach()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Intel")
    foreach(t IN LISTS TEST_EXES)
      set_target_properties(${t} PROPERTIES CXX_STANDARD 14)
    endforeach()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    if(ENABLE_CODE_ANALYSIS)
      foreach(t IN LISTS TEST_EXES)
        target_compile_options(${t} PRIVATE /analyze /WX)
      endforeach()
    endif()
endif()

if(WIN32)
    foreach(t IN LISTS TEST_EXES)
      target_compile_definitions(${t} PRIVATE _WIN32_WINNT=${WINVER})
    endforeach()

    if(DISABLE_MSVC_ITERATOR_DEBUGGING)
      foreach(t IN LISTS TEST_EXES)
        target_compile_definitions(${t} PRIVATE _ITERATOR_DEBUG_LEVEL=0)
      endforeach()
    endif()
endif()

if(VCPKG_TOOLCHAIN)
    find_package(directxmesh CONFIG REQUIRED COMPONENTS library utils)
    find_package(directxtex CONFIG REQUIRED)
    target_link_libraries(xtuvatlas PRIVATE Microsoft::DirectXTex Microsoft::DirectXMesh Microsoft::DirectXMesh::Utilities)
else()
    set(BUILD_TOOLS OFF)
    set(BUILD_TESTING OFF)
    set(BUILD_SAMPLE OFF)
    add_subdirectory(../../DirectXMesh ${CMAKE_BINARY_DIR}/directxmesh)
    add_subdirectory(../../DirectXTex ${CMAKE_BINARY_DIR}/directxtex)
    target_link_libraries(xtuvatlas PRIVATE DirectXTex DirectXMesh Utilities)
endif()
