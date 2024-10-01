#include <iostream>
#include <vector>
#include <concepts>
template <class T>
class [[maybe_unused]] DefaultDelete {
public:
    void operator()(T *p) const {
        delete p;
    }
};

template <>
class [[maybe_unused]] DefaultDelete<FILE> {
public:
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
    explicit Unique_ptr() : my_ptr(nullptr) {}; // 默认构造函数
    explicit Unique_ptr(T* p) noexcept : my_ptr(p) {}; // 自定义构造函数

    template<class U, class UDelete> requires (std::convertible_to<U *, T *>)
    explicit Unique_ptr(Unique_ptr<U, UDelete> &&that) noexcept : my_ptr(that.my_ptr) {
        that.my_ptr = nullptr;
    }

    Unique_ptr(Unique_ptr const& that) = delete;
    Unique_ptr &operator=(Unique_ptr const &that) = delete;

    Unique_ptr(Unique_ptr &&that)  noexcept {
        /*
         * 等同于:
         * my_ptr = that.my_ptr;
         * that.my_ptr = nullptr;
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

    T* release() { return exchange(my_ptr, nullptr); }

    T& operator*() const { return *my_ptr; }

    T* operator->() const { return my_ptr; }
};

template<class T, class Delete>
class Unique_ptr<T[], Delete> : Unique_ptr<T, Delete> {};

template<class T, class ...Args> // 支持多个参数传递
Unique_ptr<T> makeUnique(Args&&... args) {
    std::remove_extent_t<T> a;
    return Unique_ptr<T>(new T(std::forward<Args>(args)...));
}

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
    std::vector<Unique_ptr<Animal>> animals;
    animals.emplace_back(makeUnique<Dog>());
    animals.emplace_back(makeUnique<Cat>());
    for (auto const &a: animals) {
        a->speak();
    }
}