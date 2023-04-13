cmake_minimum_required(VERSION 3.8)

project("Project2_Server")

set(SRC_FILES
  src/main.cpp
  src/group.cpp
  src/server.cpp
  src/user.cpp
)

set(INCLUDE_FILES
  headers/group.h
  headers/server.h
  headers/user.h
)

set_property(TARGET ${CMAKE_PROJECT_NAME} PROPERTY CXX_STANDARD 20)

add_executable(${CMAKE_PROJECT_NAME} ${SRC_FILES} ${INCLUDE_FILES})
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE headers)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE wsock32 ws2_32)
