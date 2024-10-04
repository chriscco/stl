#include <iostream>
#include <atomic>
#include <memory>
#include "../unique_pointer/uniquePointer.hpp"

struct SpControlBlock {
private:
    /** 保存一共有多少指针共享当前的地址 */
    std::atomic<long> ref_count;
public:
    SpControlBlock() noexcept : ref_count(1) {};

    SpControlBlock(SpControlBlock&& that) = delete;

    /**
     * 这里使用@code{std::memory_order_relaxed}允许指令重排
     */
    void incref() {
        ref_count.fetch_add(1, std::memory_order_relaxed);
    }
    /**
     * 这里使用@code{std::memory_order_relaxed}允许指令重排
     */
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
    [[no_unique_address]] Deleter deleter;

    explicit SpControlBlockImpl(T* ptr) : my_ptr(ptr){};

    explicit SpControlBlockImpl(T* ptr_, Deleter deleter_) : my_ptr(ptr_), deleter(std::move(deleter_)){};

    ~SpControlBlockImpl() override {
        deleter(my_ptr);
    }
};

template <class T>
class SharedPointer {
private :
    T *my_ptr;
    SpControlBlock* control_b;

    template<class>
    friend class SharedPointer;

    [[maybe_unused]] friend inline SharedPointer<T>
    makeSharedCounterOnce(T* ptr, SpControlBlock* controlB){};

    explicit SharedPointer(T* ptr, SpControlBlock* controlB) : my_ptr(ptr), control_b(controlB) {};

public:
    explicit SharedPointer(std::nullptr_t = nullptr) : control_b(nullptr) {};

    // 需要确保Y is_convertible_to T
    template<class Y, std::enable_if_t<std::is_convertible_v<Y*, T*>, int> = 0>
    explicit SharedPointer(Y *ptr)
    : my_ptr(ptr), control_b(new SpControlBlockImpl<Y, DefaultDeleter<Y>>(ptr)) {
        S_setEnableSharedFromThis(my_ptr, control_b);
    };

    // 需要确保Y is_convertible_to T
    template<class Y, class Deleter, std::enable_if_t<std::is_convertible_v<Y*, T*>, int> = 0>
    explicit SharedPointer(Y *ptr, Deleter deleter)
    : my_ptr(ptr), control_b(new SpControlBlockImpl<Y, Deleter>(ptr, std::move(deleter))) {
        S_setEnableSharedFromThis(my_ptr, control_b);
    };

    SharedPointer(SharedPointer const& that) noexcept
    : my_ptr(that.my_ptr), control_b(that.control_b) {
        if (control_b) control_b->incref();
    }
    template<class Y, std::enable_if_t<std::is_convertible_v<Y*, T*>, int> = 0>
    explicit SharedPointer(SharedPointer<Y> const& that) noexcept
    : my_ptr(that.my_ptr), control_b(that.control_b) {
        if (control_b) control_b->incref();
    }

    template<class Y>
    SharedPointer(SharedPointer<Y> const& that, Y* ptr) : my_ptr(ptr), control_b(that.control_b) {
        if (control_b) control_b->incref();
    }

    template<class Y, class U>
    SharedPointer(SharedPointer<U> const& that, Y* ptr) : my_ptr(ptr), control_b(that.control_b){
        if (control_b) control_b->incref();
    };

    SharedPointer(SharedPointer&& that) noexcept : my_ptr(that.my_ptr), control_b(that.control_b){
        that.control_b = nullptr;
        that.my_ptr = nullptr;
    }

    template<class Y, std::enable_if_t<std::is_convertible_v<Y*, T*>, int> = 0>
    explicit SharedPointer(SharedPointer<Y>&& that) noexcept : my_ptr(that.my_ptr), control_b(that.control_b) {
        that.control_b = nullptr;
        that.my_ptr = nullptr;
    }

    template<class Y>
    SharedPointer(SharedPointer<Y> const&& that, Y* ptr) : my_ptr(ptr), control_b(that.control_b){
        that.control_b = nullptr;
        that.my_ptr = nullptr;
    }

    /**
     * 支持Unique_ptr转为SharedPointer
     * @tparam Y
     * @tparam Deleter
     * @param ptr
     */
    template<class Y, class Deleter, std::enable_if_t<std::is_convertible_v<Y*, T*>, int> = 0>
    explicit SharedPointer(UniquePointer<Y, Deleter>&& ptr)
            : SharedPointer(ptr.release(), ptr.get_deleter()) {};

    template<class Y>
    inline friend SharedPointer<Y> S_makeSharedFused(Y *ptr, SpControlBlock *controlB) noexcept;

    /**
     * 拷贝赋值, 如果不声明SharedPointer类型名
     * 如: p1 = p0;
     * @param that
     * @return
     */
    SharedPointer& operator=(SharedPointer const& that) noexcept {
        if (this == &that) return *this;
        if (control_b) control_b->decref();

        my_ptr = that.my_ptr;
        control_b = that.control_b;
        if (control_b) control_b->incref();
        return *this;
    }

    /**
     * 移动构造的情况下只需要将对方的指针置为空
     * @param that
     * @return
     */
    SharedPointer& operator=(SharedPointer&& that) noexcept {
        if (this == &that) return *this;
        if (control_b) control_b->decref();

        my_ptr = that.my_ptr;
        control_b = that.control_b;
        that.my_ptr = nullptr;
        that.control_b = nullptr;
        return *this;
    }

    void reset() {
        if (control_b) control_b->decref();
        control_b = nullptr;
        control_b = nullptr;
    }

    template<class Y, std::enable_if_t<std::is_convertible_v<Y*, T*>, int> = 0>
    void reset(Y* ptr) {
        if (control_b) control_b->decref();
        my_ptr = nullptr;
        control_b = nullptr;
        my_ptr = ptr;
        control_b = new SpControlBlockImpl<Y, DefaultDeleter<Y>>(ptr);
    }

    template<class Y, class Deleter, std::enable_if_t<std::is_convertible_v<Y*, T*>, int> = 0>
    void reset(Y* ptr, Deleter deleter) {
        if (control_b) control_b->decref();
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

    T* operator->() const noexcept { return my_ptr; }

    /**
     * std::add_lvalue_reference_t<T> 的作用是:
     * 如果 T 不是引用类型, 它将返回 T&, 即 T 的左值引用类型
     * 如果 T 已经是一个左值引用或右值引用, 它将返回原始类型 T 本身
     * @return
     */
    std::add_lvalue_reference_t<T> operator*() const { return *(my_ptr); }
};

template<class T>
inline SharedPointer<T> S_makeSharedFused(T *ptr, SpControlBlock *controlB) noexcept {
    return SharedPointer<T>(ptr, controlB);
}
template<class T>
SharedPointer<T> makeSharedCounterOnce(T* ptr, SpControlBlock* controlB) {
    return SharedPointer<T>(ptr, controlB);
}

template<class T, class... Args>
SharedPointer<T> makeShared(Args&&... args) {
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

template <class T>
struct EnableSharedFromThis {
private:
    SpControlBlock* control_b;
protected:
    EnableSharedFromThis() noexcept : control_b(nullptr) {};

    SharedPointer<T> shared_from_this() {
        static_assert(std::is_base_of_v<EnableSharedFromThis, T>, "must be derived class");
        if (!control_b) throw std::bad_weak_ptr();
        control_b->incref();
        return S_makeSharedFused(static_cast<T *> (this), control_b);
    }

    [[nodiscard]] SharedPointer<T const> shared_from_this() const {
        static_assert(std::is_base_of_v<EnableSharedFromThis, T>, "must be derived class");
        if (!control_b) throw std::bad_weak_ptr();
        control_b->incref();
        return S_makeSharedFused(static_cast<T const *> (this), control_b);
    }

    template<class U>
    inline friend void S_setEnableSharedFromThisOwner(EnableSharedFromThis<U>*, SpControlBlock*);
};

template<class U>
inline void S_setEnableSharedFromThisOwner(EnableSharedFromThis<U>* ptr, SpControlBlock* controlB) {
    ptr->control_b = controlB;
}

/**
 * @code{std::is_base_of_v<Base, Derived>}判断是否为派生类和基类的关系
 * @tparam T
 * @param ptr
 * @param controlB
 */
template<class T, std::enable_if_t<std::is_base_of_v<EnableSharedFromThis<T>, T>, int> = 0>
void S_setEnableSharedFromThis(EnableSharedFromThis<T>* ptr, SpControlBlock* controlB) {
    S_setEnableSharedFromThisOwner(static_cast<EnableSharedFromThis<T> *>(ptr), controlB);
}

template<class T, std::enable_if_t<!std::is_base_of_v<EnableSharedFromThis<T>, T>, int> = 0>
void S_setEnableSharedFromThis(EnableSharedFromThis<T>*, SpControlBlock*) {}