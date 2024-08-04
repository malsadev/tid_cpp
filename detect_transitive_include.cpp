#include "clang-c/Index.h"
#include "index_helpers.h"
//#include "argparse/argparse.h" //will need this to parse cmdline args

//#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
//#include <unordered_set>

int main(int, char**) //omitted parameter names to temporarirly suppress unused parameter warnings
{

    std::unordered_map<std::string, std::vector<std::string>> files_map{}; //this holds files and their included headers
    std::unordered_map<std::string, std::vector<std::string>> external_calls_map{}; //this holds files and external method calls in them
    std::vector<std::string> method_decls{}; //this holds function declarations within header files
    CXIndex index = clang_createIndex(0, 0); //Create index

    std::string entry_filename{std::string("../transitive_include_example/source.cpp")};

    index_headers(files_map, entry_filename, index);

    //TODO: collect call expressions that
    //refer to only header included declarations in a given file and save them.
    //ignore call expressions of methods declared within the source file.
    //each external method call in a source file has only 3 types:
        // callExpression
        // unexposedExpression
        // declRefExpression
    index_external_calls(external_calls_map, entry_filename, index); //is it possible to reuse translation unit in previous file? probably should cache translation units

    //TODO: index method declarations in headers files included by entry_filename
    for (auto& header: files_map[entry_filename]) {
        index_method_decls(header,method_decls, index);
    }

    //TODO: check whether called functions in entry file is
    //a subset of method_decls in the first level of included header files

    //TODO: look at sample codeblocks projects in github and imitate their structure, build steps etc



    clang_disposeIndex(index);

    return 1;

}
