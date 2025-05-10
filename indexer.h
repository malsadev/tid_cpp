#ifndef INDEXER_H
#define INDEXER_H
#include "clang-c/Index.h"

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>


namespace indexer
{

struct indexer
{
    static inline bool m_verbose;
    static inline std::vector<const char*> m_clang_options;

    //internal index used by libclang
    static inline CXIndex m_index;

    //internal index of directory representation that is *.h files aware
    static inline std::unordered_map<std::string, std::vector<std::string>> working_dir_repr{};

    static inline std::unordered_map<std::string, std::vector<std::pair<std::string, std::pair<unsigned, unsigned>>>> external_calls{};
    static inline std::unordered_map<std::string, std::vector<std::string>> method_decls{};
    static inline std::unordered_set<std::string> visited_files{};


    static int index(std::string_view file_path);
};

}

#endif // INDEXER_H




