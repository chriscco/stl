#include "vector_impl.hpp"
#include <iostream>

void printVector(Vectors const& vec) {
    std::cout << "&Vector: " << &vec << std::endl;
    for (int i = 0; i < vec.size(); i++) {
        std::cout << "vec[" << i << "]: " << vec[i] << std::endl;
    }
}

int main() {
    Vectors arr(3);
    for (int i = 0; i < arr.size(); i++) {
        arr[i] = i;
    }
    arr.resize(5);
    std::cout << "--------------------" << std::endl;
    printVector(arr);
    std::cout << "--------------------" << std::endl;

    arr.resize(2);
    printVector(arr);
    std::cout << "--------------------" << std::endl;

    Vectors bar = arr;
    bar[0] = 10000;
    printVector(bar);
    std::cout << "--------------------" << std::endl;

    Vectors car;
    car = bar;
    car.resize(4);
    car[3] = 200;
    printVector(car);
    std::cout << "--------------------" << std::endl;

    arr.resize(0);
    return 0;
}