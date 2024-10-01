#include <iostream>

template <class T>
struct SpControlBlock {
    T* my_ptr;
    int ref_count; // 保存一共有多指针共享当前的地址
    explicit SpControlBlock(T* ptr) : my_ptr(ptr), ref_count(1) {};

    SpControlBlock(SpControlBlock&& that) = delete;

    ~SpControlBlock() {
        delete my_ptr;
    }
};

template <class T>
class SharedPointer {
private :
    SpControlBlock<T>* control_b;
public:
    explicit SharedPointer(T *ptr) : control_b(new SpControlBlock(ptr)) {};
    SharedPointer(SharedPointer const& that) : control_b(that.control_b) {
        control_b->ref_count++;
    }

    ~SharedPointer() {
        control_b->ref_count--;
        if (control_b->ref_count == 0) {
            delete control_b;
        }
    }

    T* get() const { return control_b->my_ptr; }

    T* operator->() const { return control_b->my_ptr; }

    T& operator*() const { return *control_b->my_ptr; }
};

template<class T, class... Args>
SharedPointer<T> makeShared(Args &&... args) {
    return SharedPointer<T>(new T(std::forward<Args>(args)...));
}

class MyClass {
public:
    int age;
    const char* name;
    explicit MyClass(int age_, const char* name_) : age(age_), name(name_) {
        std::cout << "construct" << std::endl;
    };
    ~MyClass() {
        std::cout << "deconstruct" << std::endl;
    }
};

int main() {
    SharedPointer p0 = makeShared<MyClass>(20, "kaka");
    SharedPointer p1 = p0;
    MyClass* raw = p0.get();
}