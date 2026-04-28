#include "FastHashSet.hpp"
#include "pthash.hpp"
#include "util.hpp"

using namespace pthash;
FastHashSet::FastHashSet(const std::vector<std::string> &keys, bool verbose,
                         int num_threads) {
  // initialize vector with same size as input with default values
  m_keys(keys.size());
  // build mphf
  build_configuration config;
  config.verbose = verbose;
  config.num_threads = num_threads;

  typedef single_phf<xxhash_128,   // base hasher
                     opt_bucketer, // bucketer
                     dictionary_dictionary,        // encoder type
                     true>         // minimal
      pthash_type;

  pthash_type f;

  auto timings = f.build_in_internal_memory(keys.begin(), keys.size(), config);

  // populate internal vector
  for (int i = 0; i != keys.size(); i++) {
    m_keys[f(keys[i])] = keys[i];
  }
}

bool FastHashSet::exists(const std::string &key) const {}
