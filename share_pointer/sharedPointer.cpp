#include <iostream>

template <class T>
class SharedPointer {
private :
    T* my_ptr;
public:
    explicit SharedPointer(T *ptr) : my_ptr(ptr) {};
};

class MyClass {
public:
    int age;
    explicit MyClass(int age) : age(age) {};
};

int main() {
    SharedPointer p0(new MyClass(10));
    std::cout << &p0 << std::endl;
    SharedPointer p1 = p0;
    std::cout << &p1 << std::endl;
}