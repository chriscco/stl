#include <iostream>
#include <atomic>

template <class T>
class [[maybe_unused]] DefaultDeleter {
public:
    void operator()(T *p) const {
        delete p;
    }
};

struct SpControlBlock {
    std::atomic<long> ref_count; // 保存一共有多少指针共享当前的地址
    explicit SpControlBlock(long) noexcept : ref_count(1) {};

    SpControlBlock(SpControlBlock&& that) = delete;

    void incref() {
        ref_count.fetch_add(1, std::memory_order_relaxed);
    }
    void decref() {
        if (ref_count.fetch_sub(1, std::memory_order_relaxed) == 1) { // fetch_sub返回的是旧值
            delete this;
        }
    }
    virtual ~SpControlBlock() = default;
};

template <class T, class Deleter>
struct SpControlBlockImpl : SpControlBlock {
    T* my_ptr;
    Deleter deleter;

    explicit SpControlBlockImpl(T* ptr) : my_ptr(ptr){};

    explicit SpControlBlockImpl(T* ptr_, Deleter deleter_) : my_ptr(ptr_), deleter(std::move(deleter_)){};

    ~SpControlBlockImpl() override {
        delete my_ptr;
    }
};

template <class T>
class SharedPointer {
private :
    T* my_ptr;
    SpControlBlock* control_b;
public:
    explicit SharedPointer(std::nullptr_t = nullptr) : control_b(nullptr) {};

    template<class Y>
    explicit SharedPointer(Y *ptr = nullptr)
    : my_ptr(ptr), control_b(new SpControlBlockImpl<Y, DefaultDeleter<Y>>(ptr)) {};

    template<class Y, class Deleter>
    explicit SharedPointer(Y *ptr, Deleter deleter)
    : control_b(new SpControlBlockImpl<Y, Deleter>(ptr, std::move(deleter))) {};

    explicit SharedPointer(SharedPointer const& that) : control_b(that.control_b) {
        control_b->incref();
    }

    ~SharedPointer() {
        control_b->decref();
    }

    T* get() const { return my_ptr; }

    T* operator->() const { return my_ptr; }

    T& operator*() const { return *(my_ptr); }
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
}