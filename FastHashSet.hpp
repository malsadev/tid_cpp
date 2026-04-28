#pragma once

#include <string>
#include <vector>

class FastHashSet {
public:
    FastHashSet(const std::vector<std::string>& keys, bool verbose, int num_threads);
    bool exists(const std::string& key) const;

private:
    std::vector<std::string> m_keys;
};
