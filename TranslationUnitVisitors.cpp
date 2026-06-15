#include "TranslationUnitVisitors.hpp"
#include "spdlog/fmt/ranges.h"
#include "spdlog/spdlog.h"
#include <clang-c/CXFile.h>
#include <clang-c/CXString.h>
#include <clang-c/Index.h>

CXChildVisitResult visitMainCursor(CXCursor cursor, CXCursor /*parent*/,
                                   CXClientData client_data) {
  auto *data = static_cast<MainCursorData *>(client_data);

  CXFile cursor_file;
  clang_getExpansionLocation(clang_getCursorLocation(cursor), &cursor_file,
                             nullptr, nullptr, nullptr);
  if (!clang_File_isEqual(cursor_file, data->main_file))
    return CXChildVisit_Continue;

  CXCursor referenced = clang_getCursorReferenced(cursor);
  if (clang_Cursor_isNull(referenced))
    return CXChildVisit_Recurse;

  CXFile ref_file;
  clang_getExpansionLocation(clang_getCursorLocation(referenced), &ref_file,
                             nullptr, nullptr, nullptr);
  if (clang_File_isEqual(ref_file, data->main_file))
    return CXChildVisit_Recurse;

  CXString spelling = clang_getCursorSpelling(referenced);
  std::string symbol = clang_getCString(spelling);
  clang_disposeString(spelling);

  CXString kind_spelling = clang_getCursorKindSpelling(clang_getCursorKind(referenced));
  spdlog::debug("Processing symbol: {} of type: {}", symbol, clang_getCString(kind_spelling));
  if (!symbol.empty() && !data->symbols->exists(symbol)) {
    spdlog::error("'{}' is not declared by any direct include "
                  "(transitive symbol)",
                  symbol);
    data->failed = true;
  }

  return CXChildVisit_Recurse;
}

CXChildVisitResult visitVarDeclCursor(CXCursor cursor, CXCursor /*parent*/,
                                      CXClientData client_data) {
  auto *data = static_cast<MainCursorData *>(client_data);

  CXFile cursor_file;
  clang_getExpansionLocation(clang_getCursorLocation(cursor), &cursor_file,
                             nullptr, nullptr, nullptr);
  if (!clang_File_isEqual(cursor_file, data->main_file))
    return CXChildVisit_Continue;

  if (clang_getCursorKind(cursor) != CXCursor_VarDecl)
    return CXChildVisit_Recurse;

  CXCursor type_decl = clang_getTypeDeclaration(clang_getCursorType(cursor));
  if (clang_Cursor_isNull(type_decl))
    return CXChildVisit_Recurse;

  CXString spelling = clang_getCursorSpelling(type_decl);
  std::string symbol = clang_getCString(spelling);
  clang_disposeString(spelling);

  spdlog::debug("VarDecl type: {}", symbol);
  if (!symbol.empty() && !data->symbols->exists(symbol)) {
    spdlog::error("'{}' is not declared by any direct include "
                  "(transitive symbol)",
                  symbol);
    data->failed = true;
  }

  return CXChildVisit_Recurse;
}

CXChildVisitResult visitHeaderCursor(CXCursor cursor, CXCursor parent,
                                     CXClientData client_data) {
  auto *data = static_cast<HeaderCursorData *>(client_data);

  CXFile cursor_file;
  clang_getExpansionLocation(clang_getCursorLocation(cursor), &cursor_file,
                             nullptr, nullptr, nullptr);

  if (!clang_File_isEqual(cursor_file, data->file))
    return CXChildVisit_Continue;

  CXCursorKind kind = clang_getCursorKind(cursor);
  if (kind == CXCursor_FunctionDecl || kind == CXCursor_TypedefDecl ||
      kind == CXCursor_StructDecl || kind == CXCursor_ClassDecl ||
      kind == CXCursor_MacroDefinition) {
    CXString spelling = clang_getCursorSpelling(cursor);
    data->symbols->push_back(clang_getCString(spelling));
    clang_disposeString(spelling);
  }
  return CXChildVisit_Recurse;
}

void visitInclusion(CXFile included_file, CXSourceLocation *inclusion_stack,
                    unsigned include_len, CXClientData client_data) {
  if (include_len != 1)
    return;

  CXString included_file_name = clang_getFileName(included_file);
  auto *inclusion_data = static_cast<InclusionVisitorData *>(client_data);

  spdlog::debug("Visiting Inclusion: {} with header symbols: {}", clang_getCString(included_file_name), *inclusion_data->symbols);
  clang_disposeString(included_file_name);

  HeaderCursorData header_data{inclusion_data->symbols, included_file};
  clang_visitChildren(clang_getTranslationUnitCursor(inclusion_data->tu),
                      visitHeaderCursor, &header_data);
}
