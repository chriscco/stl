#include <iostream>
#include <atomic>
#include <thread>
template <class T>
class [[maybe_unused]] DefaultDeleter {
public:
    void operator()(T *p) const {
        delete p;
    }
};

struct SpControlBlock {
    std::atomic<long> ref_count; // 保存一共有多少指针共享当前的地址
    SpControlBlock() noexcept : ref_count(1) {};

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

    // 需要确保Y is_convertible_to T
    template<class Y>
    explicit SharedPointer(Y *ptr = nullptr)
    : my_ptr(ptr), control_b(new SpControlBlockImpl<Y, DefaultDeleter<Y>>(ptr)) {};

    // 需要确保Y is_convertible_to T
    template<class Y, class Deleter>
    explicit SharedPointer(Y *ptr, Deleter deleter)
    : control_b(new SpControlBlockImpl<Y, Deleter>(ptr, std::move(deleter))) {};

    SharedPointer(SharedPointer const& that) : my_ptr(that.my_ptr), control_b(that.control_b) {
        control_b->incref();
    }

    template<class Y, class U>
    SharedPointer(SharedPointer<U> const& that, Y* ptr) : my_ptr(ptr), control_b(that.control_b){
        control_b->incref();
    };

    ~SharedPointer() {
        control_b->decref();
    }

    [[nodiscard]] T* get() const { return my_ptr; }

    T* operator->() const { return my_ptr; }

    std::add_lvalue_reference_t<T> operator*() const { return *(my_ptr); }
};

template<class T, class... Args>
SharedPointer<T> makeShared(Args &&... args) {
    return SharedPointer<T>(new T(std::forward<Args>(args)...));
}

/**
 * 将T类型指针转换成U类型指针
 * @tparam T
 * @tparam U
 * @param ptr
 * @return
 */
template<class T, class U>
SharedPointer<T> staticPointerCast(SharedPointer<U> const &ptr) {
    return SharedPointer<T>(ptr, static_cast<T *>(ptr.get()));
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
    SharedPointer<MyClass> p0 = makeShared<MyClass>(12, "kaka");
    SharedPointer<MyClass> p1(new MyClass(19, "pp"), [](MyClass* p) { delete p; });
    SharedPointer<MyClass> p2 = p0;
    std::cout << &p0 << std::endl;
    std::cout << &p2 << std::endl;
    return 0;
}