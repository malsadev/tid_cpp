#include <string>
#include <vector>
#include <cassert>

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
    // build_fast_hashset(keys, verbosity, num_threads)
    auto store = FastHashSet(strings, true, 1);
    // assert store not null (what do constructors return)
    return 0;
  } else if (op == "build_large_hashset_using_multiple_threads") {
    auto strings = make_strings(1'000'000);
    // build_fast_hashset(keys, verbosity, num_threads)
    auto store = FastHashSet(strings, true, 3);
    // assert store not null (what do constructors return)
    return 0;
  } else if (op == "positive_negative_lookup") {
    auto strings = make_strings(1'000'000);
    // build_fast_hashset(keys, verbosity, num_threads)
    auto store = FastHashSet(strings, true, 1);
    // assert store::exists("some_string"), false
    // assert store::exists("some_other_string"), true
    return 0;
  } else if (op == "get_internal_array_size") {
    auto strings = make_strings(100);
    // build_fast_hashset(keys, verbosity, num_threads)
    auto store = FastHashSet(strings, true, 1);
    // assert store::get_size(), 100
    assert(store::get_size() == 100);
    return 0;
  }

  return -1;
}
