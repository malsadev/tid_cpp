#include "color.h"
#include "material.h"
#include "scene.h"

int main() {
    Color sky(0.5f, 0.7f, 1.0f);
    Color ground(0.2f, 0.2f, 0.2f);

    Material chrome(0.0f, 1.0f);
    Material matte(0.9f, 0.0f);

    Scene scene(sky, chrome);
    scene.add_object();

    Color blended = sky.blend(ground, 0.5f);

    // Vec3 used directly, but vec3.h is never included — comes in transitively via color.h
    Vec3 offset(1.0f, 0.0f, 0.0f);
    Vec3 shifted = blended.rgb + offset;
    (void)shifted;

    return 0;
}
