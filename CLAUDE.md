# tid_cpp

Detects transitive include violations: symbols a source file uses that come only from transitively included headers, not any direct `#include`.

## Build & test

```bash
cmake -S . -B build -DCMAKE_CXX_COMPILER=clang++   # add -DCMAKE_BUILD_TYPE=Debug for debug logs
cmake --build build
./build/tid --file path/to/file.cpp
./build/Tests/UnitTestTid
```

## Layout

- `main.cpp` — CLI (argparse) and the `--file` pipeline.
- `TranslationUnitVisitors.*` — libclang visitors: `visitInclusion` (direct headers), `visitHeaderCursor` (symbols in one header), `visitVarDeclCursor` (checks VarDecl types). `visitMainCursor` is the fuller v1 variant.
- `FastHashSet.*` — pthash minimal perfect hash for O(1) symbol lookup.

## Pipeline (`--file`)

Parse source → collect symbols from depth-1 includes → build `FastHashSet` → walk source AST, flag any VarDecl type not in the set.

## Deps

libclang (system), pthash (submodule), spdlog/argparse/Catch2 (FetchContent).

## Notes

- Don't commit `compile_commands.json` (build artifact).
- `examples/` holds test scenarios — see `examples/CLAUDE.md`.
