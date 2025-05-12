#include "complex_math.h"
#include <iostream>


int multiply (int a, int b) {
    int res = add(a, b);
    int product = a * b;
    return product;
}

int divide(int a, int b) {
    int res = subtract(a, b);

    if (b == 0) {
        std::cerr << "Can not divide by 0" << std::endl;
    }
    double quotient = divide(a, b);
    return quotient;
}
