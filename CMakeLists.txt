cmake_minimum_required(VERSION 4.0)

set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(tid_cpp)

add_executable(tid)

target_sources(tid
	PRIVATE
	main.cpp
)

find_package(Clang REQUIRED)

target_include_directories(tid PRIVATE ${CLANG_INCLUDE_DIRS})

target_link_libraries(tid PRIVATE libclang)

add_subdirectory(pthash)
target_link_libraries(tid PRIVATE PTHASH)
