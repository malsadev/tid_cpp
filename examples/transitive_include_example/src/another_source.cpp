#include <iostream>
#include "basic_math.h"

int something() {
    int a = 10;
    int b = 3;

    int sum = add(a, b);
    int difference = subtract(a, b);

    std::cout << "Sum: " << sum << std::endl;
    std::cout << "Difference: " << difference << std::endl;

    return 0;
}
