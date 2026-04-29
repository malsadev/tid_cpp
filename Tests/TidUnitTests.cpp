#include <string>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include "FastHashSet.hpp"

static std::vector<std::string> make_strings(size_t count) {
  std::vector<std::string> result;
  result.reserve(count);
  for (size_t i = 0; i < count; ++i)
    result.push_back("key_" + std::to_string(i));
  return result;
}

TEST_CASE("build_large_hashset", "[fasthashset]") {
  auto strings = make_strings(1'000'000);
  auto store = FastHashSet(strings, true, 1);
  REQUIRE(store.get_size() == 1'000'000);
}

TEST_CASE("build_large_hashset_using_multiple_threads", "[fasthashset]") {
  auto strings = make_strings(1'000'000);
  auto store = FastHashSet(strings, true, 3);
  REQUIRE(store.get_size() == 1'000'000);
}

TEST_CASE("positive_negative_lookup", "[fasthashset]") {
  auto strings = make_strings(1'000'000);
  auto store = FastHashSet(strings, true, 1);
  REQUIRE_FALSE(store.exists("some_string"));
  REQUIRE(store.exists("key_0"));
}

TEST_CASE("get_internal_array_size", "[fasthashset]") {
  auto strings = make_strings(100);
  auto store = FastHashSet(strings, true, 1);
  REQUIRE(store.get_size() == 100);
}
