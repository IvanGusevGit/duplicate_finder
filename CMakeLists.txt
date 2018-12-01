cmake_minimum_required(VERSION 3.12)

project(dupllicate_finder)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}  -fsanitize=address,undefined")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

find_package(Qt5Widgets REQUIRED)

add_executable(duplicate_finder main.cpp main_widget.h main_widget.cpp process.h process.cpp synchronized_scanner.h synchronized_scanner.cpp)

qt5_use_modules(duplicate_finder Widgets)