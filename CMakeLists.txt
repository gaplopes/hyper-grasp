cmake_minimum_required(VERSION 3.10)

project(hyper-grasp)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set the build type to Release by default
if(NOT CMAKE_BUILD_TYPE)
  # set(CMAKE_BUILD_TYPE Debug)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Print the build type
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Warning flags
set(HYPER_GRASP_CXX_WARN_FLAGS "")
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  list(APPEND HYPER_GRASP_CXX_WARN_FLAGS
    -Wall
    -Wextra
    -Wpedantic
    # -pedantic-errors
    # -Wold-style-cast
    # -Wundef
    # -Wredundant-decls
    # -Wcast-qual
    # -Wmissing-include-dirs
    # -Wdisabled-optimization
    # -Wconversion
    # -Wdouble-promotion
    # -Wshadow
    # -Wnull-dereference
  )
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    list(APPEND HYPER_GRASP_CXX_WARN_FLAGS
      -Wvector-operation-performance
      -Wduplicated-cond
    )
  endif()
else()
  message(WARNING "Unknown compiler (not GNU, Clang, or MSVC), not setting any warning flags.")
  set(HYPER_GRASP_CXX_WARN_FLAGS "")
endif()

# Find dependencies
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/modules")

# Include FetchContent module
include(FetchContent)

find_package(CLI11 QUIET)
if (NOT CLI11_FOUND)
  include(FetchContent)
  FetchContent_Declare(
    cli11
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11
    GIT_TAG        v2.4.2
  )
  FetchContent_MakeAvailable(cli11)
endif()

# find_package(mooutils QUIET)
# # If mooutils not found fallback to github
# if (NOT mooutils_FOUND)
#   include(FetchContent)
#   FetchContent_Declare(
#     mooutils
#     GIT_REPOSITORY https://github.com/adbjesus/mooutils.git
#     GIT_TAG main
#   )
#   FetchContent_MakeAvailable(mooutils)
#   add_library(mooutils::mooutils ALIAS mooutils)
# endif()

# Add include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/problem)
include_directories(${CMAKE_SOURCE_DIR}/solver)
include_directories(${CMAKE_SOURCE_DIR}/apps/include)

# Add the executable and specify the source files
add_executable(hyper-grasp
  ${CMAKE_SOURCE_DIR}/apps/main.cpp
  # Add other source files if necessary
)

# Link the executable with the libraries
target_link_libraries(hyper-grasp
  PRIVATE
    # mooutils::mooutils
    CLI11::CLI11
)

# Set compile options
target_compile_options(hyper-grasp PRIVATE ${MOBKP_CXX_WARN_FLAGS})

# Set the output name of the executable
set_target_properties(hyper-grasp PROPERTIES OUTPUT_NAME hyper-grasp)

# Install the target
install(TARGETS hyper-grasp)
