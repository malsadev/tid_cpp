#pragma once
#include "pthash.hpp"
#include <string>
#include <vector>

using namespace pthash;
typedef single_phf<xxhash_128,            // base hasher
                   opt_bucketer,          // bucketer
                   dictionary_dictionary, // encoder type
                   true>                  // minimal
    pthash_type;

class FastHashSet {
public:
  FastHashSet(const std::vector<std::string> &keys, bool verbose,
              int num_threads);
  bool exists(const std::string &key) const;
  int get_size();

private:
  std::vector<std::string> m_keys;
  pthash_type hash_func;
};
