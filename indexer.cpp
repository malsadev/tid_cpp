#include "indexer.h"
#include "clang-c/Index.h"


#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <iostream> //need to have autoinclude plugin in codeblocks

namespace   //why are empty namespaces used
{
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

void index_external_calls_helper(std::unordered_map<std::string_view, std::vector<std::string>>& external_calls_map,
                                 const std::string_view entry_filename,
                                 std::unordered_map<std::string, std::unordered_set<CXCursorKind>>& token_map)
{

    std::vector<std::string> external_method_calls{};
    external_calls_map[entry_filename] = external_method_calls;
    for (auto token_type_pair : token_map)
    {

        std::string cursor_spelling = std::get<0>(token_type_pair);
        std::unordered_set<CXCursorKind> type_set = std::get<1>(token_type_pair);


        if (type_set.size() == 3 && type_set.find(CXCursor_CallExpr) != type_set.end()
                && type_set.find(CXCursor_DeclRefExpr) != type_set.end()
                && type_set.find(CXCursor_UnexposedExpr) != type_set.end()
           )
        {
            external_calls_map[entry_filename].push_back(cursor_spelling);
        }

    }


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

int index_external_calls(const std::string_view entry_filename,
                         CXIndex& index,
                         std::unordered_map<std::string_view,
                         std::vector<std::string>> &external_calls_map
                         )
{
    std::cerr << "indexing external methods calls in: " << entry_filename << std::endl;

    CXTranslationUnit unit = clang_parseTranslationUnit(
                                 index,
                                 std::string(entry_filename).c_str(), nullptr, 0,
                                 nullptr, 0,
                                 CXTranslationUnit_None); //Parse "file.cpp"

    if (unit == nullptr)
    {
        std::cerr << "Unable to parse translation unit. Quitting.\n";
        return 0;
    }

    std::vector<std::string> external_method_calls{}; //

    std::unordered_map<std::string, std::unordered_set<CXCursorKind>> token_map{};

    CXCursor cursor = clang_getTranslationUnitCursor(unit);
    clang_visitChildren(
        cursor,
        [](CXCursor current_cursor, CXCursor parent, CXClientData client_data)
    {
        //do not expand includes, ignore cursors in include for now
        if (clang_Location_isFromMainFile(clang_getCursorLocation(current_cursor)) == 0)
        {
            return CXChildVisit_Continue;
        }




        CXCursorKind cursor_kind = clang_getCursorKind(current_cursor);
        CXString kind_CXSpelling = clang_getCursorKindSpelling(cursor_kind);
        std::string kind_spelling{clang_getCString(kind_CXSpelling)};
        std::cout << "Cursor Kind: " << kind_spelling << std::endl;
        clang_disposeString(kind_CXSpelling);

        CXString cursor_CXSpelling = clang_getCursorSpelling(current_cursor);
        std::string cursor_spelling{clang_getCString(cursor_CXSpelling)};
        std::cout << "Cursor Spelling: " << cursor_spelling << std::endl;
        clang_disposeString(cursor_CXSpelling);

//        if (cursor_spelling.empty()) {
//             return CXChildVisit_Continue;
//        }


        std::unordered_map<std::string, std::unordered_set<CXCursorKind>>* token_map = (std::unordered_map<std::string,
                std::unordered_set<CXCursorKind>>*) client_data;

        if (token_map ->  find(cursor_spelling) == token_map -> end())   //cursor spelling doesn't exist
        {
            std::unordered_set<CXCursorKind> types_set{cursor_kind};
            token_map -> insert({cursor_spelling, types_set});
        }
        else
        {
            (token_map -> at(cursor_spelling)).insert(cursor_kind);
        }

        std::cout << "\n";
        return CXChildVisit_Recurse;
    },
    &token_map
    );


    index_external_calls_helper(external_calls_map, entry_filename, token_map);

//    files_map[entry_file_name] = included_headers;

//    for(auto &header: included_headers)
//    {
//        if (files_map.find(header) == files_map.end())
//        {
//            index_headers(files_map, header, index);
//        }
//    }

    std::cerr << "Completed indexing external calls in: " << entry_filename << std::endl;
    clang_disposeTranslationUnit(unit);
    return 1;

}


int indexer::index(std::string_view file_path)
{

    if (index_headers(file_path, indexer::m_index, indexer::working_dir_repr) == 1)   //succefully travested directory from entry_filename
    {
        //there is a problem with string_view as keys, I don't understand the state of the program
        std::unordered_map<std::string_view, std::vector<std::string>> tmp = indexer::working_dir_repr;
        std::cout << "Succefully indexed headers for filename: " << file_path << std::endl;
    }
    else     //error with indexing headers
    {
        std::cerr << "Failed to index headers for filename: " << file_path << std::endl;
        return 0; // fail fast
    }

    if(index_external_calls(file_path, indexer::m_index, indexer::external_calls) == 1)  //is it possible to reuse translation unit in previous file? probably should cache translation units
    {
         std::cout << "Succefully indexed headers for filename: " << file_path << std::endl;

    } else
    {
        std::cerr << "Failed to index headers for filename: " << file_path << std::endl;
        return 0; // fail fast
    }

    indexer::working_dir_repr;
    indexer::external_calls;


}
}
