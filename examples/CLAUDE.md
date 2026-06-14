# Examples — Transitive Include Detector Test Scenarios

This directory contains C++ projects used as test inputs for the Transitive Include Detector (TID) tool. Each scenario is a self-contained mini-project designed to exercise a specific detection case.

The tool uses libclang to parse source files and identify symbols that a translation unit uses without directly including the header that defines them (i.e., they arrive only via transitive includes).

## Scenario structure convention

Each scenario is a directory with:
- One `.cpp` source file (`main.cpp`)
- A small set of headers

Scenarios should be compilable standalone with `g++ -std=c++17`.

---

## Scenario 1 — Most dependencies direct, one transitive

**Directory:** `scenario_1/`

**Files:**
- `vec3.h` — defines `Vec3` (x, y, z floats + operators)
- `color.h` — defines `Color` (stores a `Vec3`); **includes `vec3.h`**
- `material.h` — defines `Material` (roughness, metallic); no project header includes
- `scene.h` — defines `Scene`; includes `color.h` and `material.h`
- `main.cpp` — directly includes `color.h`, `material.h`, `scene.h`

**What it demonstrates:**
`main.cpp` uses `Vec3` directly (instantiates it, calls operators) but never includes `vec3.h`. `Vec3` arrives transitively via `color.h → vec3.h`. All other symbols (`Color`, `Material`, `Scene`) are covered by direct includes.

**Expected TID output:** one transitive include violation — `Vec3` used in `main.cpp` without a direct include of `vec3.h`.

---

## Scenarios 2–10

Not yet implemented.
