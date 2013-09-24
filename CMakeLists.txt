cmake_minimum_required(VERSION 2.8.11)

project(editor)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5Widgets)

set(SRCS
  src/Document.cpp
  src/Editor.cpp
)

add_executable(test src/test.cpp ${SRCS})
qt5_use_modules(test Widgets)
