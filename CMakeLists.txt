cmake_minimum_required(VERSION 3.10)
project(datarecorder)

find_package(Python COMPONENTS Interpreter)
# Use waf to resolve dependencies
if(NOT DEFINED STEINWURF_RESOLVE)
  message(STATUS "Resolving dependencies...")
  execute_process(
    COMMAND ${Python_EXECUTABLE} waf resolve ${STEINWURF_RESOLVE_OPTIONS}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    RESULT_VARIABLE STATUS)
  if(STATUS AND NOT STATUS EQUAL 0)
    message(FATAL_ERROR "Failed: ${STATUS}")
  endif()
  set(STEINWURF_RESOLVE "${CMAKE_CURRENT_SOURCE_DIR}/resolve_symlinks")

endif()

# Include common CMake settings
include("${STEINWURF_RESOLVE}/toolchains/common_settings.cmake")

# poke dependency
if(NOT TARGET steinwurf::poke)
  add_subdirectory("${STEINWURF_RESOLVE}/poke"
  ${CMAKE_CURRENT_BINARY_DIR}/poke
  EXCLUDE_FROM_ALL)
endif()

# expected dependency
if(NOT TARGET steinwurf::expected)
  add_subdirectory("${STEINWURF_RESOLVE}/expected"
  ${CMAKE_CURRENT_BINARY_DIR}/expected
  EXCLUDE_FROM_ALL)
endif()

# verify dependency
if(NOT TARGET steinwurf::verify)
  add_subdirectory("${STEINWURF_RESOLVE}/verify"
  ${CMAKE_CURRENT_BINARY_DIR}/verify
  EXCLUDE_FROM_ALL)
endif()


# Define library
add_library(datarecorder INTERFACE)
target_compile_features(datarecorder INTERFACE cxx_std_17)
target_include_directories(datarecorder INTERFACE src/)
add_library(steinwurf::datarecorder ALIAS datarecorder)
target_link_libraries(datarecorder
  INTERFACE
    steinwurf::poke
    steinwurf::expected
    steinwurf::verify
)

# Install headers
install(
  DIRECTORY ./src/datarecorder
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include
  FILES_MATCHING
  PATTERN *.hpp)



# Is top level project?
if(${CMAKE_PROJECT_NAME} STREQUAL ${PROJECT_NAME})

  # Setup testing
  enable_testing()

  # Google Test dependency
  add_subdirectory("${STEINWURF_RESOLVE}/gtest" EXCLUDE_FROM_ALL)


  # Get all steinwurf object libraries and link directly with them.
  get_property(steinwurf_object_libraries GLOBAL
    PROPERTY steinwurf::object_libraries)

  # Build test executable
  file(GLOB_RECURSE datarecorder_test_sources ./test/*.cpp)


  add_executable(datarecorder_test ${datarecorder_test_sources})
  target_link_libraries(datarecorder_test datarecorder)
  target_link_libraries(datarecorder_test steinwurf::gtest)
  target_link_libraries(datarecorder_test ${steinwurf_object_libraries})



  add_test(NAME datarecorder_test COMMAND datarecorder_test)

endif()
