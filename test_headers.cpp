#include <clang-c/Index.h>
#include <iostream>


//typedef struct {
//  enum CXTypeKind kind;
//  void *data[2];
//} CXType;

//typedef struct {
//  enum CXCursorKind kind;
//  int xdata;
//  const void *data[3];
//} CXCursor;



//FIXME: clang does not analyse header and source files the same way. Example:

//in header files, cursor kind for namespace is varDecl. In source files, kind is namespaceDecl as expected.
//recursion doesn't work in header files


//https://stackoverflow.com/questions/10561212/parsing-namespaces-with-clang-ast-differences-in-when-including-a-header-in-ano

//heade files are parsed as c files.
//namespace duckdb_libpgquery {
//struct PGList;
//struct PGSelectStmt;
//struct PGAConst;
//struct PGAStar;
//struct PGFuncCall;
//struct PGNode;
//struct PGColumnRef;
//struct PGResTarget;
//struct PGAExpr;
//struct PGJoinExpr;
//};


//Solve the simple problem:
//3 files, one source, two headers h1 and h2
//h1 depends on a param defined in h2, therefore it includes h2
//source depends on h1 method and h2 param and/or method, but only includes h1.
//project builds and compiles succesfully, but source is dependent on h2 param and/or method without explicity including it.

//detect whether a source file is dependent on a transitive inlcude or not, in the above kind of situation


int main()
{
    CXIndex index = clang_createIndex(0, 0); //Create index

    const char* cmd_args[2] =
    {
        "-x",
        "c++"
    };
    CXTranslationUnit unit = clang_parseTranslationUnit(
                                 index,
                                 "../transitive_include_example/source.cpp", cmd_args, 2,
                                 nullptr, 0,
                                 CXTranslationUnit_None); //Parse "structs.cpp"

    if (unit == nullptr)
    {
        std::cerr << "Unable to parse translation unit. Quitting.\n";
        return 0;
    }
    CXCursor cursor = clang_getTranslationUnitCursor(unit); //Obtain a cursor at the root of the translation unit

//    finds types of fundamental elements
//    clang_visitChildren(
//        cursor,
//        [](CXCursor current_cursor, CXCursor parent, CXClientData client_data)
//    {
//        CXType cursor_type = clang_getCursorType(current_cursor);
//
//        CXString type_kind_spelling = clang_getTypeKindSpelling(cursor_type.kind);
//        std::cout << "Type Kind: " << clang_getCString(type_kind_spelling);
//        clang_disposeString(type_kind_spelling);
//
//        if(cursor_type.kind == CXType_Pointer ||                     // If cursor_type is a pointer
//                cursor_type.kind == CXType_LValueReference ||              // or an LValue Reference (&)
//                cursor_type.kind == CXType_RValueReference)                // or an RValue Reference (&&),
//        {
//            CXType pointed_to_type = clang_getPointeeType(cursor_type);// retrieve the pointed-to type
//
//            CXString pointed_to_type_spelling = clang_getTypeSpelling(pointed_to_type);     // Spell out the entire
//            std::cout << "pointing to type: " << clang_getCString(pointed_to_type_spelling);// pointed-to type
//            clang_disposeString(pointed_to_type_spelling);
//        }
//        else if(cursor_type.kind == CXType_Record)
//        {
//            CXString type_spelling = clang_getTypeSpelling(cursor_type);
//            std::cout <<  ", namely " << clang_getCString(type_spelling);
//            clang_disposeString(type_spelling);
//        }
//        std::cout << "\n";
//        return CXChildVisit_Recurse;
//    },
//    nullptr
//    );


//
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
        CXString cursor_spelling = clang_getCursorSpelling(current_cursor);

        CXString kind_spelling = clang_getCursorKindSpelling(cursor_kind);
        std::cout << "Cursor Kind: " << clang_getCString(kind_spelling) << std::endl;
        clang_disposeString(kind_spelling);

        std::cout << "Cursor Spelling: " << clang_getCString(cursor_spelling) << std::endl;
        clang_disposeString(cursor_spelling);

//        if(cursor_type.kind == CXType_Pointer ||                     // If cursor_type is a pointer
//                cursor_type.kind == CXType_LValueReference ||              // or an LValue Reference (&)
//                cursor_type.kind == CXType_RValueReference)                // or an RValue Reference (&&),
//        {
//            CXType pointed_to_type = clang_getPointeeType(cursor_type);// retrieve the pointed-to type
//
//            CXString pointed_to_type_spelling = clang_getTypeSpelling(pointed_to_type);     // Spell out the entire
//            std::cout << "pointing to type: " << clang_getCString(pointed_to_type_spelling);// pointed-to type
//            clang_disposeString(pointed_to_type_spelling);
//        }
//        else if(cursor_type.kind == CXType_Record)
//        {
//            CXString type_spelling = clang_getTypeSpelling(cursor_type);
//            std::cout <<  ", namely " << clang_getCString(type_spelling);
//            clang_disposeString(type_spelling);
//        }
        std::cout << "\n";
        return CXChildVisit_Recurse;
    },
    nullptr
    );

    clang_disposeTranslationUnit(unit);
    clang_disposeIndex(index);

    return 1;
}
