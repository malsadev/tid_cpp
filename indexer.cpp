#include "indexer.h"
#include "clang-c/Index.h"

#include "util.h"
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <algorithm>
#include <iostream> //need to have autoinclude plugin in codeblocks

namespace indexer   //why are empty namespaces used
{
struct CXClientDataWrapper
{
    const std::string& filename;
    std::vector<std::string>& included_files;
};

//TODO: suppress unused parameter warnings for this function
void inclusion_visitor(CXFile included_file,
                       [[maybe_unused]]CXSourceLocation* inclusionStack,
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

    if (indexer::m_verbose)
    {
        std::cerr << "Appending: " << included_filename << std::endl;
    }


    client_data_struc -> included_files.push_back(included_filename);

    //std::cerr << (client_data_struc -> included_files[0]) << std::endl;

    clang_disposeString(included_CXFilename); //Disposing CString
}

void index_external_calls_helper(std::unordered_map<std::string, std::vector<std::pair<std::string, std::pair<unsigned, unsigned>>>>& external_calls_map,
                                 const std::string_view entry_filename,
                                 std::unordered_map<std::string, std::pair<std::pair<unsigned, unsigned>, std::unordered_set<CXCursorKind>>>& token_map)
{

    std::vector<std::pair<std::string, std::pair<unsigned, unsigned>>> external_method_calls{};
    external_calls_map[std::string(entry_filename)] = external_method_calls;
    for (auto token_info_pair : token_map)
    {

        std::string cursor_spelling = std::get<0>(token_info_pair);

        std::pair<unsigned, unsigned> cursor_location = std::get<1>(token_info_pair).first;
        std::unordered_set<CXCursorKind> type_set = std::get<1>(token_info_pair).second;


        if (type_set.size() == 3 && type_set.find(CXCursor_CallExpr) != type_set.end()
                && type_set.find(CXCursor_DeclRefExpr) != type_set.end()
                && type_set.find(CXCursor_UnexposedExpr) != type_set.end()
           )
        {
            std::pair<std::string, std::pair<unsigned, unsigned>> cursor_context{cursor_spelling, cursor_location};
            external_calls_map[std::string(entry_filename)].push_back(cursor_context);
        }

    }


}


CXChildVisitResult cursor_visitor(CXCursor current_cursor,[[maybe_unused]] CXCursor parent, CXClientData client_data)
{
//do not expand includes, ignore cursors in include for now
    if (clang_Location_isFromMainFile(clang_getCursorLocation(current_cursor)) == 0)
    {
        return CXChildVisit_Continue;
    }




    CXCursorKind cursor_kind = clang_getCursorKind(current_cursor);
    CXString kind_CXSpelling = clang_getCursorKindSpelling(cursor_kind);
    std::string kind_spelling{clang_getCString(kind_CXSpelling)};
    //std::cout << "Cursor Kind: " << kind_spelling << std::endl;
    clang_disposeString(kind_CXSpelling);

    CXString cursor_CXSpelling = clang_getCursorSpelling(current_cursor);
    std::string cursor_spelling{clang_getCString(cursor_CXSpelling)};
    //std::cout << "Cursor Spelling: " << cursor_spelling << std::endl;
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

    //std::cout << "\n";
    return CXChildVisit_Recurse;
}

void index_method_decls_helper(std::unordered_map<std::string, std::vector<std::string>>& method_decls,
                               std::string_view file_path,
                               std::unordered_map<std::string, std::unordered_set<CXCursorKind>>& token_map)
{
//     std::vector<std::string> external_method_calls{};
//    external_calls_map[entry_filename] = external_method_calls;;
    for (auto token_type_pair : token_map)
    {

        std::string cursor_spelling = std::get<0>(token_type_pair);
        std::unordered_set<CXCursorKind> type_set = std::get<1>(token_type_pair);


        if (type_set.find(CXCursor_FunctionDecl) != type_set.end()
           )
        {
            method_decls[std::string(file_path)].push_back(cursor_spelling);
        }

    }

}

}

namespace indexer //move methods above into this namespace
{

int index_headers(std::string_view file_path, CXIndex index, std::unordered_map<std::string, std::vector<std::string>>& working_dir_repr)
{
    if (indexer::m_verbose)
    {
        std::cerr << "index_headers_called for file: " << file_path << std::endl;
    }


    CXTranslationUnit unit = clang_parseTranslationUnit(
                                 index,
                                 std::string(file_path).c_str(),
                                 indexer::m_clang_options.data(),
                                 indexer::m_clang_options.size(),
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

    working_dir_repr[std::string(file_path)] = included_headers;

    for(auto& header: included_headers)
    {
        if (working_dir_repr.find(header) == working_dir_repr.end())
        {
            index_headers(header, index, working_dir_repr);
        }
    }
    if (indexer::m_verbose)
    {
        std::cerr << "index_headers completed for file: " << file_path << std::endl;
    }


    clang_disposeTranslationUnit(unit);
    return 1;

}

int index_external_calls(const std::string_view entry_filename,
                         CXIndex index,
                         std::unordered_map<std::string, std::vector<std::pair<std::string, std::pair<unsigned, unsigned>>>> &external_calls_map
                        )
{
    if (indexer::m_verbose)
    {
        std::cerr << "indexing external methods calls in: " << entry_filename << std::endl;
    }


    CXTranslationUnit unit = clang_parseTranslationUnit(
                                 index,
                                 std::string(entry_filename).c_str(),
                                 indexer::m_clang_options.data(),
                                 indexer::m_clang_options.size(),
                                 nullptr, 0,
                                 CXTranslationUnit_None); //Parse "file.cpp"

    if (unit == nullptr)
    {
        std::cerr << "Unable to parse translation unit for file: " << entry_filename << ". Quitting.\n";
        return 0;
    }

    std::vector<std::pair<std::string, std::pair<unsigned, unsigned>>> external_method_calls{}; //

    //std::unordered_map<std::string, std::unordered_set<CXCursorKind>> token_map{};
    std::unordered_map<std::string, std::pair<std::pair<unsigned, unsigned>, std::unordered_set<CXCursorKind>>> token_map{};
    //std::unordered_map<std::string, std::vector<unsigned>>  cursor_location_map{};

    CXCursor cursor = clang_getTranslationUnitCursor(unit);
        clang_visitChildren(
        cursor,
        [](CXCursor current_cursor, [[maybe_unused]]CXCursor parent, CXClientData client_data)
    {
        //do not expand includes, ignore cursors in include for now
        if (clang_Location_isFromMainFile(clang_getCursorLocation(current_cursor)) == 0)
        {
            return CXChildVisit_Continue;
        }



        CXSourceLocation source_location = clang_getCursorLocation(current_cursor);
        unsigned line, column;
        clang_getSpellingLocation(source_location, NULL, &line, &column, NULL);

        CXCursorKind cursor_kind = clang_getCursorKind(current_cursor);
        CXString kind_CXSpelling = clang_getCursorKindSpelling(cursor_kind);
        std::string kind_spelling{clang_getCString(kind_CXSpelling)};
        //std::cout << "Cursor Kind: " << kind_spelling << std::endl;
        clang_disposeString(kind_CXSpelling);

        CXString cursor_CXSpelling = clang_getCursorSpelling(current_cursor);
        std::string cursor_spelling{clang_getCString(cursor_CXSpelling)};
        //std::cout << "Cursor Spelling: " << cursor_spelling << std::endl;
        clang_disposeString(cursor_CXSpelling);

//        if (cursor_spelling.empty()) {
//             return CXChildVisit_Continue;
//        }


        std::unordered_map<std::string, std::pair<std::pair<unsigned, unsigned>, std::unordered_set<CXCursorKind>>>* token_map = (std::unordered_map<std::string, std::pair<std::pair<unsigned, unsigned>, std::unordered_set<CXCursorKind>>>*) client_data;


        if (token_map ->  find(cursor_spelling) == token_map -> end())   //cursor spelling doesn't exist
        {
            std::unordered_set<CXCursorKind> type_set{cursor_kind};
            std::pair<unsigned, unsigned> cursor_location{line, column};
            std::pair<std::pair<unsigned, unsigned>, std::unordered_set<CXCursorKind>> cursor_info{cursor_location, type_set};

            token_map -> insert({cursor_spelling, cursor_info});
        }
        else
        {
            (token_map -> at(cursor_spelling)).second.insert(cursor_kind);
        }

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
//        }std::unordered_map<std::string, std::vector<std::string>>
//    }

    for(auto& [file, _] : indexer::working_dir_repr)
    {
        if (external_calls_map.find(file) == external_calls_map.end())
        {
            index_external_calls(file, index, external_calls_map);
        }
    }

    if (indexer::m_verbose)
    {
        std::cerr << "Completed indexing external calls in: " << entry_filename << std::endl;
    }

    clang_disposeTranslationUnit(unit);
    return 1;

}


int index_method_decls(std::string_view file_path, CXIndex index, std::unordered_map<std::string, std::vector<std::string>>& method_decls)
{
    if (indexer::m_verbose)
    {
        std::cerr << "indexing external methods calls in: " << file_path << std::endl;
    }


    CXTranslationUnit unit = clang_parseTranslationUnit(
                                 index,
                                 std::string(file_path).c_str(),
                                 indexer::m_clang_options.data(),
                                 indexer::m_clang_options.size(),
                                 nullptr, 0,
                                 CXTranslationUnit_None); //Parse "file.cpp"

    if (unit == nullptr)
    {
        std::cerr << "Unable to parse translation unit. Quitting.\n";
        return 0;
    }

//    std::vector<std::string> met; //

    std::unordered_map<std::string, std::unordered_set<CXCursorKind>> token_map{};

    CXCursor cursor = clang_getTranslationUnitCursor(unit);
    clang_visitChildren(
        cursor,
        cursor_visitor,
        &token_map
    );


    index_method_decls_helper(method_decls, file_path, token_map);

    for(auto& [file, _] : indexer::working_dir_repr)
    {
        if (method_decls.find(file) == method_decls.end())
        {
            index_method_decls(file, index, method_decls);
        }
    }

    if (indexer::m_verbose)
    {
        std::cerr << "Completed indexing method declarations in: " << file_path << std::endl;
    }

    clang_disposeTranslationUnit(unit);
    return 1;


}


int index_included_method_decls(std::string_view file_path, CXIndex index,
                                std::unordered_map<std::string, std::vector<std::string>>& method_decls,
                                std::unordered_map<std::string, std::vector<std::string>>& files_map)   //no need to do reference binding for pointer CXIndex, copying pointers is cheap
{


    for (auto& header: files_map[std::string(file_path)])
    {
        //TODO: propagate files_map to find method declearations at every level of the include dependancies
        if (index_method_decls(header, index, method_decls) == 1) //don't propagate files map just yet
        {
            std::cout << "Succefully method declarations in header: " << header << std::endl;
        }
        else
        {
            std::cerr << "Failed to index method declarations in: " << header << std::endl;
            return 0;
        }
    }

    return 1;

}

int indexer::index(std::string_view file_path)
{

    if (index_headers(file_path, indexer::m_index, indexer::working_dir_repr) == 1)   //succefully travested directory from entry_filename
    {
        //there is a problem with string_view as keys, I don't understand the state of the program
        std::unordered_map<std::string, std::vector<std::string>> tmp = indexer::working_dir_repr;
        //std::cout << "current view of working_dir_repr: " << tmp << std::endl;
        if (indexer::m_verbose)
        {
            std::cout << "Succefully indexed headers for filename: " << file_path << std::endl;
        }
    }
    else     //error with indexing headers
    {
        std::cerr << "Failed to index headers for filename: " << file_path << std::endl;
        return 0; // fail fast
    }

    //Index external calls recursively
    if(index_external_calls(file_path, indexer::m_index, indexer::external_calls) == 1)  //is it possible to reuse translation unit in previous file? probably should cache translation units
    {
        std::unordered_map<std::string, std::vector<std::pair<std::string, std::pair<unsigned, unsigned>>>> tmp = indexer::external_calls; //for some reason external calls are not loaded
        if (indexer::m_verbose)
        {
            std::cout << "Succefully indexed external calls for filename: " << file_path << std::endl;
        }


    }
    else
    {
        std::cerr << "Failed to index external calls for filename: " << file_path << std::endl;
        return 0; // fail fast
    }

    //Index included method decls recursively
    if(index_method_decls(file_path, indexer::m_index, indexer::method_decls) == 1)
    {
        std::unordered_map<std::string, std::vector<std::string>> tmp = indexer::method_decls;
        if (indexer::m_verbose)
        {
            std::cout << "Succefully indexed included method declarations for filename: " << file_path << std::endl;
        }
    }
    else
    {
        std::cerr << "Failed to index included method decls for filename: " << file_path << std::endl;
        return 0; // fail fast
    }

    std::cout << "\n";
    for (auto& [file, headers] : indexer::working_dir_repr)
    {

        std::vector<std::string> valid_methods{};
        for (auto& header : headers)
        {
            valid_methods.insert(valid_methods.end(), indexer::method_decls[header].begin(),
                                 indexer::method_decls[header].end());
        }
        bool transitives = false;
        std::vector<std::pair<std::string, std::pair<unsigned, unsigned>>> transitive_deps{};
        for (auto& call_expr_context : indexer::external_calls[file])
        {
            if (std::find(valid_methods.begin(), valid_methods.end(), call_expr_context.first) == valid_methods.end())
            {
                transitive_deps.push_back(call_expr_context);
                transitives = true; //ugly, I'm resetting it to same value
            }

        }

        if (transitives)
        {
            set_color(31);
            std::cerr << "file: " << file << " depends on the following transitives:" << std::endl;

            std::cout << "\n";
            for (auto& transitive: transitive_deps)
            {
                std::cerr << transitive.first;
                printf(" on line %u, column %u\n", transitive.second.first, transitive.second.second);

            }
            reset_color();
        }
        else
        {
             set_color(32);  // Set text color to green

            std::cout << "file: " << file << " is transitive dependency free!" << std::endl;
             reset_color();
        }
    }






    return 1;

}
}
