#ifndef COLOR_UTILS_H
#define COLOR_UTILS_H

#include <iostream>

// Function to set text color using ANSI escape codes
void set_color(int color_code) {
    std::cout << "\033[" << color_code << "m";
}

// Function to reset text color to default
void reset_color() {
    std::cout << "\033[0m";
}

#endif // COLOR_UTILS_H
