cmake_minimum_required(VERSION 3.8)

project("Project2_Server")

set(SRC_FILES
  src/main.cpp
)

set(INCLUDE_FILES

)

add_executable(server ${SRC_FILES} ${INCLUDE_FILES})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET server PROPERTY CXX_STANDARD 20)
endif()
