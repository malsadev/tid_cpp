
#include <iostream>
#include "complex_math.h"

int main() {
    int x = 10, y = 5;
//    int add_res = add(x, y);
//    int substract_res = subtract(x, y);
    int product = multiply(x, y);
    float quotient = divide(x, y);

//    std::cout << "Add: " << add_res << std::endl;
//    std::cout << "Subtract: " << substract_res << std::endl;

    std::cout << "multiply: " << product << std::endl;
    std::cout << "divide " << quotient << std::endl;

    return 0;
}
