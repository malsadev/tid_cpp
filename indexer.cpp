#include "indexer.h"
#include "clang-c/Index.h"


#include <string_view>
#include <unordered_map>
#include <vector>
#include <string>
#include <iostream> //need to have autoinclude plugin in codeblocks

namespace { //why are empty namespaces used
struct CXClientDataWrapper
{
    const std::string& filename;
    std::vector<std::string>& included_files;
};

//TODO: suppress unused parameter warnings for this function
void inclusion_visitor(CXFile included_file,
                       CXSourceLocation* inclusionStack,
                       unsigned include_len,
                       CXClientData client_data)
{
    if (include_len != 1)
    {
        return;
    }

    CXClientDataWrapper* client_data_struc = (CXClientDataWrapper*) client_data;

    std::string parent_filename = client_data_struc -> filename;

    CXString included_CXFilename = clang_getFileName(included_file);
    std::string included_filename = std::string(clang_getCString(included_CXFilename));


    if (included_filename.find(parent_filename, 0) != std::string::npos)
    {
        std::cerr << "Skipping parent filename" << std::endl;
        return;
    }

    std::cerr << "Appending: " << included_filename << std::endl;
    client_data_struc -> included_files.push_back(included_filename);

    std::cerr << (client_data_struc -> included_files[0]) << std::endl;

    clang_disposeString(included_CXFilename); //Disposing CString
}
}

namespace indexer
{

int index_headers(std::string_view file_path, CXIndex index, std::unordered_map<std::string_view, std::vector<std::string>>& working_dir_repr)
{
    std::cerr << "index_headers_called for file: " << file_path << std::endl;

    CXTranslationUnit unit = clang_parseTranslationUnit(
                                 index,
                                 std::string(file_path).c_str(), nullptr, 0,
                                 nullptr, 0,
                                 CXTranslationUnit_None); //Parse "file.cpp"

    if (unit == nullptr)
    {
        std::cerr << "Unable to parse translation unit. Quitting.\n";
        return 0;
    }

    std::vector<std::string> included_headers{}; //contains header files in entry_file...maybe you only need std::string_view...test

    CXClientDataWrapper client_data_wrapper{file_path.data(), included_headers};


    clang_getInclusions(
        unit,
        &inclusion_visitor, //why omitting the & symbol also works....bring inclusion visitor
        &client_data_wrapper //and bring client_wrapper or define it within this file
    );

    working_dir_repr[file_path] = included_headers;

    for(auto& header: included_headers)
    {
        if (working_dir_repr.find(header) == working_dir_repr.end())
        {
            index_headers(header, index, working_dir_repr);
        }
    }

    std::cerr << "index_headers completed for file: " << file_path << std::endl;
    clang_disposeTranslationUnit(unit);
    return 1;

}

int indexer::index(std::string_view file_path)
{

    if (index_headers(file_path, indexer::m_index, indexer::working_dir_repr) == 1)   //succefully travested directory from entry_filename
    {
        return 1;
    }
    else     //error with indexing headers
    {
        return 0;
    }

    //index method declarations

}

}
