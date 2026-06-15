cmake_minimum_required(VERSION 4.0)

set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
option(BUILD_TESTING "Enable testing and build tests" ON)

project(tid_cpp)

include(FetchContent)
FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG        v1.15.1
)
FetchContent_Declare(
  argparse
  GIT_REPOSITORY https://github.com/p-ranav/argparse.git
  GIT_TAG        v3.2
)
FetchContent_MakeAvailable(spdlog argparse)

add_executable(tid)

find_package(Clang REQUIRED)
target_link_libraries(tid PRIVATE libclang fasthashset translation_unit_visitors argparse)

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

add_library(translation_unit_visitors)
target_sources(translation_unit_visitors
	PRIVATE TranslationUnitVisitors.cpp
	PUBLIC FILE_SET HEADERS
		BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
		FILES TranslationUnitVisitors.hpp
)
target_link_libraries(translation_unit_visitors PUBLIC libclang spdlog::spdlog fasthashset)
target_include_directories(translation_unit_visitors PUBLIC ${CLANG_INCLUDE_DIRS})


if (BUILD_TESTING)
	enable_testing()
	add_subdirectory(Tests)
endif()

add_subdirectory(pthash)
target_link_libraries(fasthashset PUBLIC PTHASH) # PUBLIC because FastHashSet.hpp includes pthash.hpp, so consumers also need PTHASH headers
