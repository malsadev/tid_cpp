cmake_minimum_required(VERSION 4.0)

set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(tid_cpp)

add_executable(tid)

find_package(Clang REQUIRED)
target_link_libraries(tid PRIVATE libclang fasthashset)

target_link_libraries(tid PRIVATE libclang)

target_sources(tid
	PRIVATE
	main.cpp
)

add_library(fasthashset)
target_sources(fasthashset
	PRIVATE FastHashSet.cpp
	PUBLIC FILE_SET HEADERS
		BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
		FILES FastHashSet.hpp
)

add_subdirectory(pthash)
target_link_libraries(fasthashset PRIVATE PTHASH)
# target_link_libraries(tid PRIVATE PTHASH)
