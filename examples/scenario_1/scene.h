#pragma once

#include "color.h"
#include "material.h"

struct Scene {
    Color background;
    Material default_material;
    int object_count;

    Scene(Color background, Material default_material)
        : background(background), default_material(default_material), object_count(0) {}

    void add_object() { ++object_count; }
};
