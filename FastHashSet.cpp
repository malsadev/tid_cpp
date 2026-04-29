#include "FastHashSet.hpp"
#include "pthash.hpp"
#include "util.hpp"
#include <string>
#include <vector>

using namespace pthash;
FastHashSet::FastHashSet(const std::vector<std::string> &keys, bool verbose,
                         int num_threads) {
  // initialize vector with same size as input with default values
  m_keys = std::vector(keys.size(), std::string("default"));
  // build mphf
  build_configuration config;
  config.verbose = verbose;
  config.num_threads = num_threads;

  hash_func.build_in_internal_memory(keys.begin(), keys.size(), config);

  // populate internal vector
  for (uint64_t i = 0; i != keys.size(); i++) {
    m_keys[hash_func(keys[i])] = keys[i];
  }
}

bool FastHashSet::exists(const std::string &key) const {
  if (key == m_keys[hash_func(key)]) {
    return true;
  }
  return false;
}

int FastHashSet::get_size() { return m_keys.size(); }
