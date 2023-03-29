cmake_minimum_required(VERSION 3.8)

project("Project2_Server")

set(SRC_FILES
  src/main.cpp
)

set(INCLUDE_FILES

)

find_package(Boost 1.80.0 REQUIRED)

add_executable(server ${SRC_FILES} ${INCLUDE_FILES})
target_link_libraries(server PRIVATE Boost::boost)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET server PROPERTY CXX_STANDARD 20)
endif()
