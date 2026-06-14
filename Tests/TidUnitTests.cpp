#include <algorithm>
#include <clang-c/Index.h>
#include <cstring>
#include <string>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include "FastHashSet.hpp"
#include "TranslationUnitVisitors.hpp"
#include "catch2/catch_message.hpp"

static std::vector<std::string> make_strings(size_t count) {
  std::vector<std::string> result;
  result.reserve(count);
  for (size_t i = 0; i < count; ++i)
    result.push_back("key_" + std::to_string(i));
  return result;
}

TEST_CASE("build_large_hashset", "[fasthashset]") {
  auto strings = make_strings(1'000'000);
  auto store = FastHashSet(strings, true, 1);
  REQUIRE(store.get_size() == 1'000'000);
}

TEST_CASE("build_large_hashset_using_multiple_threads", "[fasthashset]") {
  auto strings = make_strings(1'000'000);
  auto store = FastHashSet(strings, true, 3);
  REQUIRE(store.get_size() == 1'000'000);
}

TEST_CASE("positive_negative_lookup", "[fasthashset]") {
  auto strings = make_strings(1'000'000);
  auto store = FastHashSet(strings, true, 1);
  REQUIRE_FALSE(store.exists("some_string"));
  REQUIRE(store.exists("key_0"));
}

TEST_CASE("get_internal_array_size", "[fasthashset]") {
  auto strings = make_strings(100);
  auto store = FastHashSet(strings, true, 1);
  REQUIRE(store.get_size() == 100);
}

// ---------------------------------------------------------------------------
// Helpers for visitor tests
// ---------------------------------------------------------------------------

static bool containsSymbol(const SymbolList &list, const std::string &sym) {
  return std::find(list.begin(), list.end(), sym) != list.end();
}

static CXTranslationUnit parseMemory(CXIndex index, const char *filename,
                                     const char *source,
                                     unsigned flags = CXTranslationUnit_None) {
  CXUnsavedFile unsaved{filename, source, (unsigned long)strlen(source)};
  return clang_parseTranslationUnit(index, filename, nullptr, 0, &unsaved, 1,
                                    flags);
}

static CXTranslationUnit parseMemory(CXIndex index, const char *filename,
                                     CXUnsavedFile *unsaved, unsigned count,
                                     unsigned flags = CXTranslationUnit_None) {
  return clang_parseTranslationUnit(index, filename, nullptr, 0, unsaved,
                                    count, flags);
}

// ---------------------------------------------------------------------------
// visitHeaderCursor tests
// ---------------------------------------------------------------------------

TEST_CASE("collect_function_declarations", "[header_cursor]") {
  CXIndex index = clang_createIndex(0, 0);
  CXTranslationUnit unit =
      parseMemory(index, "fake.hpp", "int foo(); double bar();");
  REQUIRE(unit != nullptr);

  SymbolList symbols;
  HeaderCursorData data{&symbols, clang_getFile(unit, "fake.hpp")};
  clang_visitChildren(clang_getTranslationUnitCursor(unit), visitHeaderCursor,
                      &data);

  REQUIRE(containsSymbol(symbols, "foo"));
  REQUIRE(containsSymbol(symbols, "bar"));

  clang_disposeTranslationUnit(unit);
  clang_disposeIndex(index);
}

TEST_CASE("collect_typedefs", "[header_cursor]") {
  CXIndex index = clang_createIndex(0, 0);
  CXTranslationUnit unit =
      parseMemory(index, "fake.hpp", "typedef float MyFloat;");
  REQUIRE(unit != nullptr);

  SymbolList symbols;
  HeaderCursorData data{&symbols, clang_getFile(unit, "fake.hpp")};
  clang_visitChildren(clang_getTranslationUnitCursor(unit), visitHeaderCursor,
                      &data);

  REQUIRE(containsSymbol(symbols, "MyFloat"));

  clang_disposeTranslationUnit(unit);
  clang_disposeIndex(index);
}

TEST_CASE("collect_struct_types", "[header_cursor]") {
  CXIndex index = clang_createIndex(0, 0);
  CXTranslationUnit unit =
      parseMemory(index, "fake.hpp", "struct Point { int x; int y; };");
  REQUIRE(unit != nullptr);

  SymbolList symbols;
  HeaderCursorData data{&symbols, clang_getFile(unit, "fake.hpp")};
  clang_visitChildren(clang_getTranslationUnitCursor(unit), visitHeaderCursor,
                      &data);

  REQUIRE(containsSymbol(symbols, "Point"));

  clang_disposeTranslationUnit(unit);
  clang_disposeIndex(index);
}

TEST_CASE("ignores_transitive_symbols", "[header_cursor]") {
  CXIndex index = clang_createIndex(0, 0);
  const char *headerB = "int bar();";
  const char *headerA = "#include \"fake_b.hpp\"\nint foo();";
  CXUnsavedFile unsaved[] = {
      {"fake_a.hpp", headerA, (unsigned long)strlen(headerA)},
      {"fake_b.hpp", headerB, (unsigned long)strlen(headerB)},
  };
  CXTranslationUnit unit = parseMemory(index, "fake_a.hpp", unsaved, 2);
  REQUIRE(unit != nullptr);

  SymbolList symbols;
  HeaderCursorData data{&symbols, clang_getFile(unit, "fake_a.hpp")};
  clang_visitChildren(clang_getTranslationUnitCursor(unit), visitHeaderCursor,
                      &data);

  REQUIRE(containsSymbol(symbols, "foo"));
  REQUIRE_FALSE(containsSymbol(symbols, "bar"));

  clang_disposeTranslationUnit(unit);
  clang_disposeIndex(index);
}

// ---------------------------------------------------------------------------
// visitInclusion tests
// ---------------------------------------------------------------------------

TEST_CASE("builds_symbol_list_from_direct_include", "[inclusion]") {
  CXIndex index = clang_createIndex(0, 0);
  const char *header_src = "int foo(); double bar();";
  const char *main_src = "#include \"/fake_header.hpp\"";
  CXUnsavedFile unsaved[] = {
      {"/fake_main.cpp", main_src, (unsigned long)strlen(main_src)},
      {"/fake_header.hpp", header_src, (unsigned long)strlen(header_src)},
  };
  CXTranslationUnit unit = parseMemory(index, "/fake_main.cpp", unsaved, 2);
  REQUIRE(unit != nullptr);

  SymbolList symbols;
  InclusionVisitorData data{&symbols, unit};
  clang_getInclusions(unit, visitInclusion, &data);

  CAPTURE(symbols);
  REQUIRE(containsSymbol(symbols, "foo"));
  REQUIRE(containsSymbol(symbols, "bar"));

  clang_disposeTranslationUnit(unit);
  clang_disposeIndex(index);
}

TEST_CASE("empty_header_produces_no_symbols", "[inclusion]") {
  CXIndex index = clang_createIndex(0, 0);
  const char *header_src = "";
  const char *main_src = "#include \"/fake_empty.hpp\"";
  CXUnsavedFile unsaved[] = {
      {"/fake_main.cpp", main_src, (unsigned long)strlen(main_src)},
      {"/fake_empty.hpp", header_src, (unsigned long)strlen(header_src)},
  };
  CXTranslationUnit unit = parseMemory(index, "/fake_main.cpp", unsaved, 2);
  REQUIRE(unit != nullptr);

  SymbolList symbols;
  InclusionVisitorData data{&symbols, unit};
  clang_getInclusions(unit, visitInclusion, &data);

  REQUIRE(symbols.empty());

  clang_disposeTranslationUnit(unit);
  clang_disposeIndex(index);
}

// ---------------------------------------------------------------------------
// visitMainCursor tests (skeletons — depend on FastHashSet client_data shape)
// ---------------------------------------------------------------------------

TEST_CASE("known_symbol_produces_no_report", "[main_cursor]") {
  // TODO: build FastHashSet from a fake header's symbols
  // TODO: parse a fake main file that uses a symbol from the hashset
  // TODO: run visitMainCursor and verify no diagnostic is emitted
  SKIP();
}

TEST_CASE("transitive_symbol_produces_report", "[main_cursor]") {
  // TODO: build FastHashSet from a fake header's symbols
  // TODO: parse a fake main file that uses a symbol NOT in the hashset
  // TODO: run visitMainCursor and verify a diagnostic is emitted
  SKIP();
}
