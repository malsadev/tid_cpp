include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.8.1
)
FetchContent_MakeAvailable(Catch2)

add_executable(UnitTestTid)

target_sources(UnitTestTid
  PRIVATE
    TidUnitTests.cpp
)

target_link_libraries(UnitTestTid PRIVATE fasthashset Catch2::Catch2WithMain)

include(Catch)
catch_discover_tests(UnitTestTid)
