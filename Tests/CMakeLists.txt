
add_executable(UnitTestTid)


target_sources(UnitTestTid
  PRIVATE
    TidUnitTests.cpp
)

target_link_libraries(UnitTestTid PRIVATE MathFunctions)


function(MathFunctionTest op)
  add_test(
    NAME ${op}
    COMMAND TestMathFunctions ${op}
  )
endfunction()

MathFunctionTest(add)
MathFunctionTest(mul)
MathFunctionTest(sqrt)
MathFunctionTest(sub)
