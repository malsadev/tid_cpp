#include <argparse/argparse.hpp>
#include <clang-c/Index.h>
#include <iostream>

int main(int argc, char *argv[]) {
  argparse::ArgumentParser program("tid");

  program.add_argument("--file", "-f")
      .help("single source file to analyze");

  program.add_argument("--dir", "-d")
      .help("directory to analyze recursively");

  try {
    program.parse_args(argc, argv);
  } catch (const std::exception &e) {
    std::cerr << e.what() << "\n\n" << program;
    return 1;
  }
}
