#include "uniquePointer.h"
#include <iostream>
#include <utility>
template <class T>
struct DefaultDelete {
    void operator()(T *p) const {
        delete p;
    }
};

template<>
struct DefaultDelete<FILE> {
    void operator()(FILE *file) {
        fclose(file);
    }
};

template <class T, class Delete = DefaultDelete<T>>
class Unique_ptr {
private:
    T* my_ptr;
public:
    Unique_ptr() : my_ptr(nullptr) {}; // 默认构造函数
    explicit Unique_ptr(T* p) : my_ptr(p) {}; // 自定义构造函数

    Unique_ptr(Unique_ptr const& that) = delete;
    Unique_ptr &operator=(Unique_ptr const &that) = delete;

    Unique_ptr(Unique_ptr &&that)  noexcept {
        /*
         * 等同于:
         * my_ptr = that.my_ptr;
         * that.my_ptr = nullptr
         */
        my_ptr = std::exchange(that.my_ptr, nullptr);
    }

    Unique_ptr& operator=(Unique_ptr && that)  noexcept {
        if (this != &that) [[likely]]{ // 防止用instance移动赋值构造instance本身
            if (my_ptr) Delete{}(my_ptr);

            my_ptr = std::exchange(that.my_ptr, nullptr);
        }
        return *this;
    }

    ~Unique_ptr() {
        if (my_ptr) Delete{}(my_ptr);
    }

    T* get() const { return my_ptr; }

    T& operator*() const { return *my_ptr; }

    T* operator->() const { return my_ptr; }
};

class MyClass {
public:
    int a, b, c;

    MyClass(int a, int b, int c) : a(a), b(b), c(c) {};
};
template <class T, class ...Args> // 支持多个参数传递
Unique_ptr<T> make_unique(Args... args) {
    return Unique_ptr<T>(new T(args...));
}
int main() {
    auto p = make_unique<MyClass>(1, 2, 3);
    std::cout << "a: " << (*p).a << std::endl;
}