cmake_minimum_required(VERSION 2.8.11)

project(editor)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5Widgets)

set(SRCS
  src/Document.cpp
  src/Editor.cpp
)

add_library(editor ${SRCS})
qt5_use_modules(editor Widgets)
set_target_properties(editor PROPERTIES AUTOMOC TRUE)

add_executable(test src/test.cpp)
target_link_libraries(test editor)
