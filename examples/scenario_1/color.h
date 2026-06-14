#pragma once

#include "vec3.h"

struct Color {
    Vec3 rgb;

    Color(float r, float g, float b) : rgb(r, g, b) {}

    Color blend(const Color& other, float t) const {
        return Color(
            rgb.x + (other.rgb.x - rgb.x) * t,
            rgb.y + (other.rgb.y - rgb.y) * t,
            rgb.z + (other.rgb.z - rgb.z) * t
        );
    }
};
