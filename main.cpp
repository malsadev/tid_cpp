#include <clang-c/Index.h>

// #include "pthash.hpp"
#include <iostream>

int main() {
  CXIndex index = clang_createIndex(0, 0); // Create index
  CXTranslationUnit unit =
      clang_parseTranslationUnit(index, "file.cpp", nullptr, 0, nullptr, 0,
                                 CXTranslationUnit_None); // Parse "file.cpp"

  if (unit == nullptr) {
    std::cerr << "Unable to parse translation unit. Quitting.\n";
    return 0;
  }
  CXCursor cursor = clang_getTranslationUnitCursor(
      unit); // Obtain a cursor at the root of the translation unit
  // using namespace pthash;
  //
  // /* Generate 1M random 64-bit keys as input data. */
  // static const uint64_t num_keys = 1'000'000;
  // static const uint64_t seed = essentials::get_random_seed();
  // std::cout << "generating input data..." << std::endl;
  // auto keys = distinct_uints<uint64_t>(num_keys, seed);
  // // Can also use:
  // // auto keys = distinct_strings(num_keys, seed);
  // assert(keys.size() == num_keys);
}
