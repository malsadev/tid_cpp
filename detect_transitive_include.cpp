#include "clang-c/Index.h"
#include "include/index_helpers.h"
#include "include/argparse.hpp" //will need this to parse cmdline args

//#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
//#include <unordered_set>
#include <unistd.h>

int main(int argc, char* argv[]) //omitted parameter names to temporarirly suppress unused parameter warnings
{

    auto is_stdout = isatty(STDOUT_FILENO) == 1;
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);

    argparse::ArgumentParser program("transitive_include_detector", "0.1.0");

    program.add_argument("path").remaining();

    //Generic program information.. didn't need to define a help argument because it is already taken care
    // of by the arg parser library. I don't even have an action for the argument.
//    program.add_argument("-h", "--help")
//    .help("Shows help message and exits")
//    .default_value(true)
//    .implicit_value(true);



    program.add_argument("-l", "--language")
    .default_value<std::string>(std::string {"c++"})
    .help("Language option used by clang");

    program.add_argument("--std")
    .default_value<std::string>(std::string {"c++17"})
    .help("C++ standard to be used by clang");

//    std::cerr << program << std::endl;

    try
    {
        program.parse_args(argc, argv);
    }
    catch (const std::runtime_error& err)
    {
        if (program.get<bool>("--help"))
        {
            std::cerr << program << std::endl;
            return 0;
        }
        else
        {
            std::cerr << err.what() << std::endl;
            std::cerr << program << std::endl;
            std::exit(1);
        }
    }

//    std::unordered_map<std::string, std::vector<std::string>> files_map{}; //this holds files and their included headers
//    std::unordered_map<std::string, std::vector<std::string>> external_calls_map{}; //this holds files and external method calls in them
//    std::vector<std::string> method_decls{}; //this holds function declarations within header files
//    CXIndex index = clang_createIndex(0, 0); //Create index
//
//    std::string entry_filename{std::string("../transitive_include_example/source.cpp")};
//
//    index_headers(files_map, entry_filename, index);
//
//    //TODO: collect call expressions that
//    //refer to only header included declarations in a given file and save them.
//    //ignore call expressions of methods declared within the source file.
//    //each external method call in a source file has only 3 types:
//    // callExpression
//    // unexposedExpression
//    // declRefExpression
//    index_external_calls(external_calls_map, entry_filename, index); //is it possible to reuse translation unit in previous file? probably should cache translation units
//
//    //TODO: index method declarations in headers files included by entry_filename
//    for (auto& header: files_map[entry_filename])
//    {
//        index_method_decls(header,method_decls, index);
//    }
//
//    //TODO: check whether called functions in entry file is
//    //a subset of method_decls in the first level of included header files
//
//    //TODO: look at sample codeblocks projects in github and imitate their structure, build steps etc
//
//
//
//    clang_disposeIndex(index);

    return 1;

}
