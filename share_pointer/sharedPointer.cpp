#include <iostream>
#include <atomic>
#include <thread>
#include "../unique_pointer/uniquePointer.hpp"

struct SpControlBlock {
private:
    std::atomic<long> ref_count; // 保存一共有多少指针共享当前的地址
public:
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

    long refcount() {
        return ref_count.load();
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
        deleter(my_ptr);
    }
};

template <class T>
class SharedPointer {
private :
    T* my_ptr;
    SpControlBlock* control_b;

    template<class>
    friend class SharedPointer;

public:
    explicit SharedPointer(std::nullptr_t = nullptr) : my_ptr(nullptr), control_b(nullptr) {};

    // 需要确保Y is_convertible_to T
    template<class Y>
    explicit SharedPointer(Y *ptr = nullptr)
    : my_ptr(ptr), control_b(new SpControlBlockImpl<Y, DefaultDeleter<Y>>(ptr)) {};

    // 需要确保Y is_convertible_to T
    template<class Y, class Deleter>
    explicit SharedPointer(Y *ptr, Deleter deleter)
    : my_ptr(ptr), control_b(new SpControlBlockImpl<Y, Deleter>(ptr, std::move(deleter))) {};

    SharedPointer(SharedPointer const& that) : my_ptr(that.my_ptr), control_b(that.control_b) {
        control_b->incref();
    }

    template<class Y>
    SharedPointer(SharedPointer<Y> const& that, Y*ptr) : my_ptr(ptr), control_b(that.control_b) {
        control_b->incref();
    }

    template<class Y, class U>
    SharedPointer(SharedPointer<U> const& that, Y* ptr) : my_ptr(ptr), control_b(that.control_b){
        control_b->incref();
    };

    SharedPointer(SharedPointer&& that) noexcept {
        that.control_b = nullptr;
        that.my_ptr = nullptr;
    }

    template<class Y>
    SharedPointer(SharedPointer<Y> const&& that, Y* ptr) : my_ptr(ptr), control_b(that.control_b){
        that.control_b = nullptr;
        that.my_ptr = nullptr;
    }

    /**
     * 拷贝赋值, 如果不声明SharedPointer类型名
     * 如: p1 = p0;
     * @param that
     * @return
     */
    SharedPointer& operator=(SharedPointer const& that) {
        if (this == &that) return *this;

        my_ptr = that.my_ptr;
        control_b = that.control_b;
        return *this;
    }

    void reset() {
        control_b->decref();
        control_b = nullptr;
    }

    template<class Y>
    void reset(Y* ptr) {
        control_b->decref();
        my_ptr = nullptr;
        control_b = nullptr;
        my_ptr = ptr;
        control_b = new SpControlBlockImpl<Y, DefaultDeleter<Y>>(ptr);
    }

    template<class Y, class Deleter>
    void reset(Y* ptr, Deleter deleter) {
        control_b->decref();
        my_ptr = nullptr;
        control_b = nullptr;
        my_ptr = ptr;
        control_b = new SpControlBlockImpl<Y, Deleter>(ptr, std::move(deleter));
    }

    ~SharedPointer() {
        if (control_b) control_b->decref();
    }

    long use_count() {
        return control_b ? control_b->refcount() : 0;
    }

    bool unique() {
        return control_b == nullptr || control_b->refcount() == 1;
    }

    /**
     * 返回指针所管理的原始地址
     * @return
     */
    [[nodiscard]] T* get() const noexcept { return my_ptr; }

    T* operator->() const { return my_ptr; }

    /**
     * std::add_lvalue_reference_t<T> 的作用是:
     * 如果 T 不是引用类型, 它将返回 T&, 即 T 的左值引用类型
     * 如果 T 已经是一个左值引用或右值引用, 它将返回原始类型 T 本身
     * @return
     */
    std::add_lvalue_reference_t<T> operator*() const { return *(my_ptr); }
};

template<class T, class... Args>
SharedPointer<T> makeShared(Args &&... args) {
    return SharedPointer<T>(new T(std::forward<Args>(args)...));
}

/**
 * 将T类型指针转换成U类型指针
 * 用于在相关类型之间进行安全的转换
 * 比如基本类型, 指针类型和类层次之间的转换
 * @tparam T
 * @tparam U
 * @param ptr
 * @return
 */
template<class T, class U>
SharedPointer<T> staticPointerCast(SharedPointer<U> const &ptr) {
    return SharedPointer<T>(ptr, static_cast<T *>(ptr.get()));
}

/**
 * 用于在有 const 或 volatile 限定符的类型之间进行转换, 以去掉这些限定符
 * 只能添加或移除 const 或 volatile 限定符
 * @tparam T
 * @tparam U
 * @param ptr
 * @return
 */
template<class T, class U>
SharedPointer<T> constPointerCast(SharedPointer<U> const &ptr) {
    return SharedPointer<T>(ptr, const_cast<T *>(ptr.get()));
}
/**
 * 用于低级别的强制转换, 通常在指针类型之间或将一个类型的指针转换为另一个不相关的类型
 * 不进行任何类型检查
 * @tparam T
 * @tparam U
 * @param ptr
 * @return
 */
template<class T, class U>
SharedPointer<T> reinterpretPointerCast(SharedPointer<U> const &ptr) {
    return SharedPointer<T>(ptr, reinterpret_cast<T *>(ptr.get()));
}

/**
 * 主要用于进行类层次之间的安全转换，特别是在多态情况下（即使用虚函数的类）。
 * @tparam T
 * @tparam U
 * @param ptr
 * @return
 */
template<class T, class U>
SharedPointer<T> dynamicPointerCast(SharedPointer<U> const &ptr) {
    T* p = dynamic_cast<T *>(ptr.get());
    if (p) {
        return SharedPointer<T>(ptr, p);
    } else return nullptr;
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

class MyClassDerived : public MyClass {
    explicit MyClassDerived(int age, const char* name) : MyClass(age, name) {
        std::cout << "MyClass Derived Construct\n";
    };

    ~MyClassDerived() {
        std::cout << "MyClass Derived Destruct\n";
    }
};

int main() {
    SharedPointer<MyClass> p0 = makeShared<MyClass>(12, "kaka");
    SharedPointer<MyClass> p1(new MyClass(19, "pp"), [](MyClass* p) { delete p; });
    SharedPointer<MyClass> p2 = p0;

    std::cout << "--------------------------------" << std::endl;
    std::cout << "p0.get(): " << p0.get() << std::endl;
    std::cout << "p1.get(): " << p1.get() << std::endl;
    std::cout << "p2.get(): " << p2.get() << std::endl;

    p2 = p1; // 拷贝赋值

    std:: cout << "age: " << staticPointerCast<MyClassDerived>(p0).operator*().age << std::endl;
    std:: cout << "name: " << staticPointerCast<MyClassDerived>(p0).operator*().name << std::endl;
    std::cout << "p0: " << &p0 << std::endl;
    std::cout << "p1: " << &p1 << std::endl;
    std::cout << "p2: " << &p2 << std::endl;

    std::cout << "--------------------------------" << std::endl;
    std::cout << "p0.get(): " << p0.get() << std::endl;
    std::cout << "p1.get(): " << p1.get() << std::endl;
    std::cout << "p2.get(): " << p2.get() << std::endl;
    return 0;
}