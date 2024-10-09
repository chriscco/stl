#pragma once

#include <vector>
#include <iostream>
#include <cstring>
#include <memory>
#include <utility>
#include <initializer_list>

template<class T, class Alloc = std::allocator<T>>
class Vectors {
private:
    [[no_unique_address]] Alloc allocator;

    T* m_data;
    size_t m_size;
    size_t m_capacity;

public:
    Vectors() : m_data(nullptr), m_size(0), m_capacity(0){};

    explicit Vectors(size_t size) {
        m_data = allocator.allocate(size);
        m_size = size;
        m_capacity = size;
        for (size_t i = 0; i < m_size; i++) {
            std::construct_at(&m_data[i]);
        }
    }

    explicit Vectors(size_t size, T const& val) {
        m_data = allocator.allocate(size);
        m_capacity = m_size = size;
        for (size_t i = 0; i < size; i++) {
            std::construct_at(&m_data[i], val);
        }
    }

    /**
     * 调用@code{explicit Vectors(InputIt first, InputIt last)}
     * @param list
     */
    Vectors(std::initializer_list<T> list) : Vectors(list.begin(), list.end()){};

    template<std::random_access_iterator InputIt>
    explicit Vectors(InputIt first, InputIt last) {
        size_t n = last - first;
        m_data = allocator.allocate(n);

        m_capacity = m_size = n;
        for (size_t i = 0; i < n; i++) {
            m_data[i] = *first;
            first++;
        }
    }

    /**
     * 深拷贝防止析构m_data两次
     * @param that
     */
    Vectors(Vectors const& that) {
        m_capacity = m_size = that.m_size;
        if (m_capacity != 0) {
            m_data = allocator.allocate(m_size);
            for (size_t i = 0; i < m_size; i++) {
                std::construct_at(&m_data[i], std::as_const(that.m_data[i]));
            }
        } else {
            m_data = nullptr;
        }
    }

    Vectors& operator=(Vectors const& that) {
        if (this == &that) return *this;

        m_capacity = m_size = that.m_size;
        if (m_capacity != 0) {
            m_data = allocator.allocate(m_size);
            for (size_t i = 0; i < m_size; i++) {
                std::construct_at(&m_data[i], std::as_const(that.m_data[i]));
            }
        }
        return *this;
    }

    Vectors(Vectors&& that) noexcept {
        // 如果移动赋值的目的地对象已经有元素了, 删除原本的元素
        if (m_capacity != 0) allocator.deallocate(m_data, m_capacity);

        m_data = that.m_data;
        m_size = that.m_size;
        m_capacity = that.m_capacity;

        that.m_data = nullptr;
        that.m_size = 0;
        that.m_capacity = 0;
    }

    Vectors& operator=(Vectors&& that) noexcept {
        if (this == &that) return *this;
        // 如果移动赋值的目的地对象已经有元素了, 删除原本的元素
        if (m_capacity != 0) allocator.deallocate(m_data, m_capacity);

        m_data = that.m_data;
        m_size = that.m_size;
        m_capacity = that.m_capacity;

        that.m_data = nullptr;
        that.m_size = 0;
        that.m_capacity = 0;
        return *this;
    }

    /**
     * 用于给向量重新赋值, 或者复制元素
     * @tparam InputIt
     * @param first
     * @param last
     */
    template<std::random_access_iterator InputIt>
    void assign(InputIt first, InputIt last) {
        clear();
        size_t n = last - first;
        reserve(n);
        m_size = n;
        for (size_t i = 0; i < n; i++) {
            std::construct_at(&m_data[i], *first);
            first++;
        }
    }

    /**
     * 用于给向量重新赋值, 或者复制元素
     * @tparam InputIt
     * @param first
     * @param last
     */
    void assign(size_t n, T const& val) {
        clear();
        reserve(n);
        m_size = n;
        for (size_t i = 0; i < n; i++) {
            std::construct_at(&m_data[i], val);
        }
    }

    /**
     * 转发给对应函数
     * @param list
     */
    void assign(std::initializer_list<T> list) {
        assign(list.begin(), list.end());
    }

    template<std::random_access_iterator InputIt>
    T* insert(T const* it, InputIt first, InputIt last) {
        size_t n = last - first, j = it - m_data;
        if (n == 0) return const_cast<T *> (it);
        reserve(m_size + n);
        for (size_t i = m_size; i > j; i--) {
            std::construct_at(&m_data[i + n - 1], std::move(m_data[i - 1]));
            std::destroy_at(&m_data[i - 1]);
        }
        m_size += n;
        for (size_t i = j; i < j + n; i++) {
            std::construct_at(&m_data[i], *first);
            first++;
        }
        return m_data + j;
    }

    T* insert(T const* it, T const& val) {
        size_t j = it - m_data;
        reserve(m_size + 1);
        for (size_t i = m_size; i > j; i--) {
            std::construct_at(&m_data[i], std::move(m_data[i - 1]));
            std::destroy_at(&m_data[i - 1]);
        }
        m_size += 1;
        std::construct_at(&m_data[j], val);
        return m_data + j;
    }

    T* insert(T const* it, T&& val) {
        size_t j = it - m_data;
        reserve(m_size + 1);
        for (size_t i = m_size; i > j; i--) {
            std::construct_at(&m_data[i], std::move(m_data[i - 1]));
            std::destroy_at(&m_data[i - 1]);
        }
        m_size += 1;
        std::construct_at(&m_data[j], std::move(val));

        return m_data + j;
    }

    T* insert(T const* it, size_t n, T const& val) {
        size_t j = it - m_data;
        if (n == 0) return const_cast<T *> (it);
        reserve(m_size + n);
        /* 将当前iterator之后的元素往后移n位 */
        for (size_t i = m_size; i > j; i--) {
            std::construct_at(&m_data[i + n - 1], std::move(m_data[i - 1]));
            std::destroy_at(&m_data[i - 1]);
        }
        m_size += n;
        /* 给指定的区间元素赋值 */
        for (size_t i = j; i < j + n; i++) {
            std::construct_at(&m_data[i], val);
        }
        return m_data + j;
    }

    /**
     * 转发给对应函数
     * @param list
     */
    T* insert(T const* it, std::initializer_list<T> list) {
        return insert(it, list.begin(), list.end());
    }

    void swap(Vectors& that) {
        std::swap(m_data, that.m_data);
        std::swap(m_size, that.m_size);
        std::swap(m_capacity, that.m_capacity);
    }

    void clear() {
        for (size_t i = 0; i < m_size; i++) {
            std::destroy_at(&m_data[i]);
        }
    }

    void resize(size_t size) {
        reserve(size);
        if (size > m_size) {
            for (size_t i = m_size; i < size; i++) {
                std::construct_at(&m_data[i]);
            }
        }
    }

    void resize(size_t size, T const& val) {
        reserve(size);
        if (size > m_size) {
            for (size_t i = m_size; i < size; i++) {
                std::construct_at(&m_data[i], val);
            }
        }
    }

    /**
     * 将capacity缩小到size
     */
    void shrink_to_fit() {
        auto old_data = m_data;
        auto old_capacity = m_capacity;

        m_capacity = m_size;
        if (m_size == 0) {
            m_data = nullptr;
        } else m_data = allocator.allocate(m_size);

        if (old_capacity) {
            for (size_t i = 0; i < m_size; i++) {
                std::construct_at(&m_data[i], std::as_const(old_data[i])); // m_data[i] = old_data[i];
            }
            allocator.deallocate(old_data, old_capacity);
        }
    }

    void reserve(size_t n) {
        if (n <= m_capacity) [[likely]] return;
        n = std::max(n, m_capacity * 2);
        auto old_data = m_data;
        auto old_capacity = m_capacity;
        if (n == 0) {
            m_data = nullptr;
            m_capacity = 0;
        } else {
            m_data = allocator.allocate(n);
            m_capacity = n;
        }
        if (old_capacity) {
            if (m_capacity != 0) {
                for (size_t i = 0; i < m_size; i++) {
                    std::construct_at(&m_data[i], std::as_const(old_data[i]));
                }
            }
            allocator.deallocate(old_data, old_capacity);
        }
    }

    [[nodiscard]] T const& at(size_t i) const {
        if (i >= m_size) throw std::out_of_range("vector::at");
        return m_data[i];
    }

    T& at(size_t i) {
        if (i >= m_size) throw std::out_of_range("vector::at");
        return m_data[i];
    }

    [[nodiscard]] T const& front() const {
        return at(0);
    }

    T& front() {
        return at(0);
    }

    [[nodiscard]] T const& back() const {
        return at(m_size - 1);
    }

    T& back() {
        return at(m_size - 1);
    }

    T* begin() {
        return m_data;
    }

    [[nodiscard]] T const *cbegin() const {
        return m_data;
    }

    T* end() {
        return m_data + m_size;
    }

    [[nodiscard]] T const *cend() const {
        return m_data + m_size;
    }

    std::reverse_iterator<T *> rbegin() {
        return std::make_reverse_iterator(m_data);
    }

    std::reverse_iterator<T *> rend() {
        return std::make_reverse_iterator(m_data + m_size);
    }

    [[nodiscard]] std::reverse_iterator<T const*> rbegin() const {
        return std::make_reverse_iterator(m_data);
    }

    [[nodiscard]] std::reverse_iterator<T const*> rend() const {
        return std::make_reverse_iterator(m_data + m_size);
    }

    [[nodiscard]] std::reverse_iterator<T const*> crbegin() const {
        return std::make_reverse_iterator(m_data);
    }

    [[nodiscard]] std::reverse_iterator<T const*> crend() const {
        return std::make_reverse_iterator(m_data + m_size);
    }


    void push_back(T const& val) {
        reserve(m_size + 1);
        std::construct_at(&m_data[m_size], val);
        m_size += 1;
    }

    void push_back(T &&val) {
        reserve(m_size + 1);
        std::construct_at(&m_data[m_size], std::move(val));
        m_size += 1;
    }

    template<class ...Args>
    T& emplace_back(Args &&... args) {
        reserve(m_size + 1);
        T *p = &m_data[m_size];
        std::construct_at(&m_data[m_size], std::forward<Args>(args)...);
        m_size += 1;
        return *p;
    }

    void erase(size_t i) {
        for (size_t j = i + 1; j < m_size; j++) {
            m_data[j - 1] = std::move(m_data[j]);
        }
        resize(m_size - 1);
    }

    void erase(size_t beg, size_t end) {
        size_t diff = end - beg;
        for (size_t j = end; j < m_size; j++) {
            m_data[j - diff] = std::move(m_data[j]);
        }
        resize(m_size - diff);
    }

    T* erase(T const* it) {
        size_t i = it - m_data;
        for (size_t j = i + 1; j < m_size; j++) {
            m_data[j - 1] = std::move(m_data[j]);
        }
        resize(m_size - 1);
        return const_cast<T *> (it + 1);
    }

    T* erase(T const* first, T const* last) {
        size_t diff = last - first;
        for (size_t j = last - m_data; j < m_size; j++) {
            m_data[j - diff] = std::move(m_data[j]);
        }
        resize(m_size - diff);
        return const_cast<T *> (last);
    }

    [[nodiscard]] size_t size() const {
        return m_size;
    }

    [[nodiscard]] size_t capacity() const {
        return m_capacity;
    }

    T const& operator[](size_t i) const {
        return m_data[i];
    }

    T& operator[](size_t i) {
        return m_data[i];
    }

    ~Vectors() { if (m_capacity != 0) allocator.deallocate(m_data, m_capacity); }
};
