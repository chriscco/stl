#include "uniquePointer.h"
#include <iostream>
#include <utility>
#include <vector>
#include <concepts>
template <class T>
struct DefaultDelete {
    void operator()(T *p) const {
        delete p;
    }
};

template <>
struct DefaultDelete<FILE> {
    void operator()(FILE *file) {
        fclose(file);
    }
};

template <class T, class U>
T exchange(T& dest, U&& newVal) {
    T temp = std::move(dest);
    dest = std::forward<U>(newVal);
    return temp;
}

template <class T, class Delete = DefaultDelete<T>>
class Unique_ptr {
private:
    T* my_ptr;
    template<class U, class UDelete>
    friend class Unique_ptr;

public:
    Unique_ptr() : my_ptr(nullptr) {}; // 默认构造函数
    explicit Unique_ptr(T* p) : my_ptr(p) {}; // 自定义构造函数

    Unique_ptr(Unique_ptr const& that) = delete;
    Unique_ptr &operator=(Unique_ptr const &that) = delete;

    template<class U, class UDelete>
    requires (std::convertible_to<U *, T *>)
    explicit Unique_ptr(Unique_ptr<U, UDelete> &&that) {
        my_ptr = std::exchange(that.my_ptr, nullptr);
    }

    Unique_ptr(Unique_ptr &&that)  noexcept {
        /*
         * 等同于:
         * my_ptr = that.my_ptr;
         * that.my_ptr = nullptr
         */
        my_ptr = exchange(that.my_ptr, nullptr);
    }

    Unique_ptr& operator=(Unique_ptr&& that)  noexcept {
        if (this != &that) [[likely]]{ // 防止用instance移动赋值构造instance本身
            if (my_ptr) Delete{}(my_ptr);

            my_ptr = exchange(that.my_ptr, nullptr);
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

template<class T, class Delete>
class Unique_ptr<T[], Delete> : Unique_ptr<T, Delete> {};

template<class T, class ...Args> // 支持多个参数传递
Unique_ptr<T> make_unique(Args... args) {
    return Unique_ptr<T>(new T(args...));
}

class Animal {
public:
    virtual void speak() = 0;
    virtual ~Animal() = default;
};

class Dog : Animal {
public:
    void speak() override { std::cout << "wang!" << std::endl; }
};

class Cat : Animal {
public:
    void speak() override { std::cout << "miao!" << std::endl; }
};

int main() {
    std::vector<Unique_ptr<Animal>> animals;
    animals.push_back(make_unique<Dog>());
    animals.push_back(make_unique<Cat>());
}