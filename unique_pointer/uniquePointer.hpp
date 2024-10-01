#include <iostream>
#include <vector>
#include <concepts>

/**
 * 创建的指针会在超出作用域时自动调用 Deleterr<T>::operator()删除这个指针
 * @tparam T
 */
template <class T>
struct DefaultDeleter {
public:
    void operator()(T *p) const {
        delete p;
    }
};

/**
 * 对于FILE类型的特化函数
 */
template <>
struct DefaultDeleter<FILE> {
public:
    void operator()(FILE *file) {
        fclose(file);
    }
};

/**
 * 等同于std::exchange();
 * 在当前实现中, 函数的目的是保证始终只有一个指针
 * @tparam T
 * @tparam U
 * @param dest
 * @param newVal
 * @return
 */
template <class T, class U>
T exchange(T& dest, U&& newVal) {
    T temp = std::move(dest);
    dest = std::forward<U>(newVal);
    return temp;
}

/**
 * unique_ptr实现
 * @tparam T
 * @tparam Deleter
 */
template <class T, class Deleter = DefaultDeleter<T>>
class Unique_ptr {
private:
    T* my_ptr;
    Deleter* deleter;

    template<class U, class UDeleter>
    friend class Unique_ptr;

public:
    explicit Unique_ptr() : my_ptr(nullptr) {}; // 默认构造函数
    explicit Unique_ptr(T* p) noexcept : my_ptr(p) {}; // 自定义构造函数

    /**
     * 移动构造函数, 同时兼容派生类对于基类的转换
     * @tparam U
     * @tparam UDeleter
     * @param that
     */
    template<class U, class UDeleter> requires (std::convertible_to<U *, T *>)
    explicit Unique_ptr(Unique_ptr<U, UDeleter> &&that) noexcept : my_ptr(that.my_ptr) {
        that.my_ptr = nullptr;
    }

    /**
     * 禁用拷贝构造
     * @param that
     */
    Unique_ptr(Unique_ptr const& that) = delete;
    Unique_ptr &operator=(Unique_ptr const &that) = delete;

    /**
     * 移动构造函数
     * @param that
     */
    Unique_ptr(Unique_ptr &&that)  noexcept {
        /*
         * 等同于:
         * my_ptr = that.my_ptr;
         * that.my_ptr = nullptr;
         */
        my_ptr = exchange(that.my_ptr, nullptr);
    }

    /**
     * 移动赋值
     * @param that
     * @return
     */
    Unique_ptr& operator=(Unique_ptr&& that)  noexcept {
        if (this != &that) [[likely]]{ // 防止用instance移动赋值构造instance本身
            if (my_ptr) Deleter{}(my_ptr);

            my_ptr = exchange(that.my_ptr, nullptr);
        }
        return *this;
    }

    ~Unique_ptr() {
        if (my_ptr) Deleter{}(my_ptr);
    }

    /**
     * get()返回指针
     * @return
     */
    [[nodiscard]] T* get() const { return my_ptr; }

    Deleter* get_deleter() const {
        return deleter;
    }

    /**
     * 等价于std::change()
     * 允许传入新的指针将my_ptr指向新的对象
     * @param p
     */
    void reset(T *p = nullptr) {
        if (my_ptr) Deleter{}(my_ptr);
        my_ptr = p;
    }

    /*
     * 防止特殊情况下对指针的两次释放
     */
    T* release() { return exchange(my_ptr, nullptr); }

    std::add_lvalue_reference_t<T> operator*() const { return *my_ptr; }

    T* operator->() const { return my_ptr; }
};

/**
 * 对于数组类型情况下的类模版,
 * @tparam T
 * @tparam Deleter
 */
template<class T, class Deleter>
class Unique_ptr<T[], Deleter> : Unique_ptr<T, Deleter> {};

/**
 * 支持多个参数传递, 包括非有界数组(vector...)
 * 该模板的第三个参数将默认为0, 使得缺少第三个参数依然可以运行
 * @tparam T
 * @tparam Args
 * @param args
 * @return
 */
template<class T, class ...Args, std::enable_if_t<!std::is_bounded_array_v<T>, int> = 0>
Unique_ptr<T> makeUnique(Args&&... args) {
    std::remove_extent_t<T> a;
    return Unique_ptr<T>(new T(std::forward<Args>(args)...));
}