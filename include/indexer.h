#ifndef INDEXER_H
#define INDEXER_H
#include "clang-c/Index.h"

#include <string_view>
#include <unordered_map>
#include <vector>
//#include <string>


namespace indexer
{

struct indexer
{
    static inline bool m_verbose;
    static inline std::vector<std::string_view> m_clang_options;

    //internal index used by libclang
    static inline CXIndex m_index;

    //internal index of directory representation that is *.h files aware
    static inline std::unordered_map<std::string_view, std::vector<std::string>> working_dir_repr{};

    static inline std::unordered_map<std::string_view, std::vector<std::string>> external_calls{};
//    static inline std::unordered_map<std::string_view, std::vector<std::string>> method_decls{};


    static int index(std::string_view file_path);
};

}









#endif // INDEXER_H




