#pragma once

struct Material {
    float roughness;
    float metallic;

    Material(float roughness, float metallic) : roughness(roughness), metallic(metallic) {}

    bool is_mirror() const { return roughness < 0.01f && metallic > 0.99f; }
};
