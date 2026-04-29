add_executable(UnitTestTid)

target_sources(UnitTestTid
  PRIVATE
    TidUnitTests.cpp
)

target_link_libraries(UnitTestTid PRIVATE fasthashset)

function(FastHashSetTest op)
  add_test(
    NAME ${op}
    COMMAND UnitTestTid ${op}
  )
endfunction()

FastHashSetTest(build_large_hashset)
FastHashSetTest(build_large_hashset_using_multiple_threads)
FastHashSetTest(positive_negative_lookup)
FastHashSetTest(get_internal_array_size)
