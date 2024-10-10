#include <iostream>

#include "sets.hpp"


int main() {
    Sets s;
    s.insert(1);
    s.insert(2);
    s.insert(3);
    s.insert(0);
    s.insert(18);
    s.insert(30);
    std::cout << s.find(3) << std::endl;
    return 0;
}