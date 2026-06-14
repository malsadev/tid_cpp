#include "TranslationUnitVisitors.hpp"
#include "spdlog/spdlog.h"

CXChildVisitResult visitMainCursor(CXCursor cursor, CXCursor parent,
                                   CXClientData client_data) {
  // query fast hashset online
  // if symbol not from main file, check if fasthashset (you can check for
  // mempership whether symbol is from main file using libclang provided APIs)
  // if found, fine
  // if not output that the symbols might be coming from a transitive (use print
  // formatter for symbol location) the client data in this callback is the
  // queryable data structure
  return CXChildVisit_Recurse;
}

CXChildVisitResult visitHeaderCursor(CXCursor cursor, CXCursor parent,
                                     CXClientData client_data) {
  auto *data = static_cast<HeaderCursorData *>(client_data);

  spdlog::info("Entering visitHeaderCursor with client_data = {}", client_data);
  CXFile cursor_file;
  clang_getExpansionLocation(clang_getCursorLocation(cursor), &cursor_file,
                             nullptr, nullptr, nullptr);

  spdlog::info("included CXFile = {}", data->file);
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
  spdlog::info("Entering visitInclusion with client_data = {}", client_data);
  auto *inclusion_data = static_cast<InclusionVisitorData *>(client_data);
  HeaderCursorData header_data{inclusion_data->symbols, included_file};

  spdlog::info("Outer translation unit = {}", (void *)inclusion_data->tu);
  clang_visitChildren(clang_getTranslationUnitCursor(inclusion_data->tu),
                      visitHeaderCursor, &header_data);
}
