#include <vector>
#include "uniquePointer.hpp"
class Animal {
public:
    virtual void speak() = 0;
    virtual ~Animal() = default;
};

struct Dog : Animal {
public:
    void speak() override { std::cout << "Dog!" << std::endl; }
};

class Cat : public Animal {
public:
    void speak() override { std::cout << "Cat!" << std::endl; }
};

int main() {
    std::vector<UniquePointer<Animal>> animals;
    animals.emplace_back(makeUnique<Dog>());
    animals.emplace_back(makeUnique<Cat>());
    for (auto const &a: animals) {
        a->speak();
    }
}