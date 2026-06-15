#include "FastHashSet.hpp"
#include "spdlog/fmt/ranges.h"
#include "TranslationUnitVisitors.hpp"
#include "spdlog/spdlog.h"
#include <argparse/argparse.hpp>
#include <array>
#include <clang-c/Index.h>
#include <iostream>
#include <string>

namespace {

bool isSourceFile(const std::string &path) {
  static const std::array<std::string, 4> exts = {".c", ".cpp", ".cc", ".cxx"};
  for (const auto &ext : exts) {
    if (path.size() >= ext.size() &&
        path.compare(path.size() - ext.size(), ext.size(), ext) == 0)
      return true;
  }
  return false;
}

} // namespace

int main(int argc, char *argv[]) {
#ifndef NDEBUG
  spdlog::set_level(spdlog::level::debug);
#endif
  argparse::ArgumentParser program("tid");

  auto &group = program.add_mutually_exclusive_group(true);
  group.add_argument("--file", "-f").help("single source file to analyze");
  group.add_argument("--dir", "-d").help("directory to analyze recursively");

  try {
    program.parse_args(argc, argv);
  } catch (const std::exception &e) {
    spdlog::error(e.what());
    std::cerr << program;
    return 1;
  }

  if (auto file = program.present("--file")) {
    const std::string &path = *file;

    if (!isSourceFile(path)) {
      spdlog::error("'{}' is not a C/C++ source file", path);
      return 1;
    }

    CXIndex index = clang_createIndex(0, 0);
    CXTranslationUnit unit = clang_parseTranslationUnit(
        index, path.c_str(), nullptr, 0, nullptr, 0, CXTranslationUnit_None);
    if (unit == nullptr) {
      spdlog::error("unable to parse '{}'", path);
      clang_disposeIndex(index);
      return 1;
    }

    SymbolList header_symbols;
    InclusionVisitorData inclusion_data{&header_symbols, unit};
    clang_getInclusions(unit, visitInclusion, &inclusion_data);

    if (header_symbols.empty()) {
      spdlog::info("source file has no direct-include symbols to check");
      clang_disposeTranslationUnit(unit);
      clang_disposeIndex(index);
      return 0;
    }

    spdlog::debug("Combined header symbols: {}", header_symbols);

    FastHashSet symbols(header_symbols);

    MainCursorData check{&symbols, clang_getFile(unit, path.c_str()), false};
    clang_visitChildren(clang_getTranslationUnitCursor(unit), visitVarDeclCursor,
                        &check);

    clang_disposeTranslationUnit(unit);
    clang_disposeIndex(index);

    if (check.failed)
      return 1;

    spdlog::info("all symbols resolved through direct includes");
  }

  return 0;
}
