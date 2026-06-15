#pragma once
#include "FastHashSet.hpp"
#include <clang-c/Index.h>
#include <string>
#include <vector>

struct SymbolLocation {
  // symbol name
  // Location
};

typedef std::string Symbol;
typedef std::vector<Symbol> SymbolList;

struct HeaderCursorData {
  SymbolList *symbols;
  CXFile file;
};

struct InclusionVisitorData {
  SymbolList *symbols;
  CXTranslationUnit tu;
};

struct MainCursorData {
  const FastHashSet *symbols;
  CXFile main_file;
  bool failed;
};

CXChildVisitResult visitMainCursor(CXCursor cursor, CXCursor parent,
                                   CXClientData client_data);

CXChildVisitResult visitVarDeclCursor(CXCursor cursor, CXCursor parent,
                                      CXClientData client_data);

CXChildVisitResult visitHeaderCursor(CXCursor cursor, CXCursor parent,
                                     CXClientData client_data);

void visitInclusion(CXFile included_file, CXSourceLocation *inclusion_stack,
                    unsigned include_len, CXClientData client_data);
