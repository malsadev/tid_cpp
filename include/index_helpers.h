#include "clang-c/Index.h"
//#include "argparse/argparse.h" //will need this to parse cmdline args

#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <unordered_set>

struct CXClientDataWrapper
{
    const std::string& filename;
    std::vector<std::string>& included_files;
};

void index_external_calls_helper(std::unordered_map<std::string, std::vector<std::string>>& external_calls_map,
                                 const std::string& entry_filename,
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


void index_method_decls_helper(std::vector<std::string>& method_decls,
//                                std::string& filename,
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
            method_decls.push_back(cursor_spelling);
        }

    }

}

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

    clang_disposeString(included_CXFilename); //Disposing CString
}

CXChildVisitResult cursor_visitor(CXCursor current_cursor, CXCursor parent, CXClientData client_data)
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
}

int index_headers(std::unordered_map<std::string, std::vector<std::string>> &files_map, const std::string& entry_file_name, CXIndex& index)
{

    std::cerr << "index_headers_called for file: " << entry_file_name << std::endl;

    CXTranslationUnit unit = clang_parseTranslationUnit(
                                 index,
                                 entry_file_name.c_str(), nullptr, 0,
                                 nullptr, 0,
                                 CXTranslationUnit_None); //Parse "file.cpp"

    if (unit == nullptr)
    {
        std::cerr << "Unable to parse translation unit. Quitting.\n";
        return 0;
    }

    std::vector<std::string> included_headers{}; //contains header files in entry_file

    CXClientDataWrapper client_data_wrapper{entry_file_name, included_headers};


    clang_getInclusions(
        unit,
        &inclusion_visitor, //why omitting the & symbol also works
        &client_data_wrapper
    );

    files_map[entry_file_name] = included_headers;

    for(auto &header: included_headers)
    {
        if (files_map.find(header) == files_map.end())
        {
            index_headers(files_map, header, index);
        }
    }

    std::cerr << "index_headers completed for file: " << entry_file_name << std::endl;
    clang_disposeTranslationUnit(unit);
    return 1;

}

//TODO: write a function to analyse external method calls in a given file and store them for later comparision
int index_external_calls(std::unordered_map<std::string,
                         std::vector<std::string>> &external_calls_map,
                         const std::string& entry_filename, CXIndex& index)
{
    std::cerr << "indexing external methods calls in: " << entry_filename << std::endl;

    CXTranslationUnit unit = clang_parseTranslationUnit(
                                 index,
                                 entry_filename.c_str(), nullptr, 0,
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

//TODO: write a function to index method delcarations in a file
int index_method_decls(std::string& filename, std::vector<std::string>& method_decls, CXIndex& index)
{
    std::cerr << "indexing external methods calls in: " << filename << std::endl;

    CXTranslationUnit unit = clang_parseTranslationUnit(
                                 index,
                                 filename.c_str(), nullptr, 0,
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


    index_method_decls_helper(method_decls, token_map);


    std::cerr << "Completed indexing method declarations in: " << filename << std::endl;
    clang_disposeTranslationUnit(unit);
    return 1;


}



//TODO: write a function that compares types present in two files

//TODO: write a function that compares types present in a parent file and types present in its header files

//TODO: port development to vs code to use cmake and vs code AI extensionss

