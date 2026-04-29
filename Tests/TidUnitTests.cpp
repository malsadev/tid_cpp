#include <string>
#include <vector>
#include <cassert>
#include "FastHashSet.hpp"

static std::vector<std::string> make_strings(size_t count) {
  std::vector<std::string> result;
  result.reserve(count);
  for (size_t i = 0; i < count; ++i)
    result.push_back("key_" + std::to_string(i));
  return result;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    return -1;
  }

  std::string op(argv[1]);

  if (op == "build_large_hashset") {
    auto strings = make_strings(1'000'000);
    auto store = FastHashSet(strings, true, 1);
    assert(store.get_size() == 1'000'000);
    return 0;
  } else if (op == "build_large_hashset_using_multiple_threads") {
    auto strings = make_strings(1'000'000);
    auto store = FastHashSet(strings, true, 3);
    assert(store.get_size() == 1'000'000);
    return 0;
  } else if (op == "positive_negative_lookup") {
    auto strings = make_strings(1'000'000);
    auto store = FastHashSet(strings, true, 1);
    assert(!store.exists("some_string"));
    assert(store.exists("key_0"));
    return 0;
  } else if (op == "get_internal_array_size") {
    auto strings = make_strings(100);
    auto store = FastHashSet(strings, true, 1);
    assert(store.get_size() == 100);
    return 0;
  }

  return -1;
}
