cmake_minimum_required(VERSION 3.8)

project("Project2_Server")

set(SRC_FILES
  src/main.cpp
  src/group.cpp
  src/server.cpp
)

set(INCLUDE_FILES
  headers/group.h
  headers/server.h
)

# find_package(Boost 1.80.0 REQUIRED)

add_executable(${CMAKE_PROJECT_NAME} ${SRC_FILES} ${INCLUDE_FILES})
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE headers)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE wsock32 ws2_32)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ${CMAKE_PROJECT_NAME} PROPERTY CXX_STANDARD 20)
endif()
