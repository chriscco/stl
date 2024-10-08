#pragma once

#include <vector>
#include <cstring>

class Vectors {
private:
    int* m_data;
    size_t m_size;
    size_t m_capacity;

public:
    Vectors() : m_data(nullptr), m_size(0), m_capacity(0){};

    explicit Vectors(size_t size) {
        m_data = new int[size];
        memset(m_data, 0, sizeof(int) * size);
        m_size = size;
        m_capacity = size;
    }

    explicit Vectors(size_t size, int val) {
        m_data = new int[size];
        for (size_t i = 0; i < size; i++) {
            m_data[i] = val;
        }
        m_size = size;
        m_capacity = size;
    }

    /**
     * 深拷贝防止析构m_data两次
     * @param that
     */
    Vectors(Vectors const& that) {
        m_size = that.m_size;
        m_capacity = that.m_capacity;
        if (m_size != 0) {
            m_data = new int[that.m_size];
            memcpy(m_data, that.m_data, m_size * sizeof(int));
        } else {
            m_data = nullptr;
        }
    }

    Vectors& operator=(Vectors const& that) {
        if (this == &that) return *this;

        clear();
        m_size = that.m_size;
        if (m_size != 0) {
            m_data = new int[m_size]{};
            memcpy(m_data, that.m_data, m_size * sizeof(int));
        }
        return *this;
    }

    Vectors(Vectors&& that) noexcept {
        m_data = that.m_data;
        m_size = that.m_size;
        m_capacity = that.m_capacity;
        that.m_data = nullptr;
        that.m_size = 0;
        that.m_capacity = 0;
    }

    Vectors& operator=(Vectors&& that) noexcept {
        clear();
        m_data = that.m_data;
        m_size = that.m_size;
        that.m_data = nullptr;
        that.m_size = 0;
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
        size_t n = last - first;
        reserve(n);
        m_size = n;
        for (int i = 0; i < n; i++) {
            m_data[i] = *first;
            first++;
        }
    }

    /**
     * 用于给向量重新赋值, 或者复制元素
     * @tparam InputIt
     * @param first
     * @param last
     */
    void assign(size_t n, int val) {
        reserve(n);
        m_size = n;
        for (int i = 0; i < n; i++) {
            m_data[i] = val;
        }
    }

    template<std::random_access_iterator InputIt>
    void insert(int const* it, InputIt first, InputIt last) {
        size_t n = last - first, j = it - m_data;
        if (n == 0) return;
        m_size += n;
        reserve(m_size);
        for (int i = (int) n; i > 0; i--) {
            m_data[j + n + i - 1] = std::move(m_data[j + i - 1]);
        }
        for (int i = (int) j; i < j + n; i++) {
            m_data[i] = *first;
            first++;
        }
    }

    void insert(int const* it, size_t n, int val) {
        reserve(n);
        m_size = n;
        for (int i = it - m_data; i < n; i++) {
            m_data[i] = val;
        }
    }

    void swap(Vectors& that) {
        std::swap(m_data, that.m_data);
        std::swap(m_size, that.m_size);
        std::swap(m_capacity, that.m_capacity);
    }

    void clear() {
        m_size = 0;
    }

    void resize(size_t size) {
        reserve(size);
        m_size = size;
    }

    void shrink_to_fit() {
        m_capacity = m_size;
        if (m_size == 0) {
            m_data = nullptr;
        } else m_data = new int[m_size];

        auto old_data = m_data;
        if (old_data) {
            memcpy(m_data, old_data, m_size * sizeof(int));
            delete[] old_data;
        }
    }

    void reserve(size_t n) {
        if (n <= m_capacity) [[likely]] return;
        n = std::max(n, m_capacity * 2);
        auto old_data = m_data;

        if (n == 0) {
            m_data = nullptr;
            m_capacity = 0;
        } else {
            m_data = new int[n];
            m_capacity = n;
        }
        if (old_data) {
            if (m_size != 0) {
                memcpy(m_data, old_data, m_size * sizeof(int));
            }
            delete[] old_data;
        }
    }

    [[nodiscard]] int const& at(size_t i) const {
        if (i >= m_size) throw std::out_of_range("vector::at");
        return m_data[i];
    }

    int& at(size_t i) {
        if (i >= m_size) throw std::out_of_range("vector::at");
        return m_data[i];
    }

    [[nodiscard]] int const& front() const {
        return at(0);
    }

    int& front() {
        return at(0);
    }

    [[nodiscard]] int const& back() const {
        return at(m_size - 1);
    }

    int& back() {
        return at(m_size - 1);
    }

    int* begin() {
        return m_data;
    }

    [[nodiscard]] int const * begin() const {
        return m_data;
    }

    int* end() {
        return m_data + m_size;
    }

    [[nodiscard]] int const *end() const {
        return m_data + m_size;
    }

    void push_back(int val) {
        reserve(m_size + 1);
        m_data[m_size] = val;
        m_size += 1;
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

    void erase(int const* it) {
        size_t i = it - m_data;
        for (size_t j = i + 1; j < m_size; j++) {
            m_data[j - 1] = std::move(m_data[j]);
        }
        resize(m_size - 1);
    }

    void erase(int const* first, int const* last) {
        size_t diff = last - first;
        for (size_t j = last - m_data; j < m_size; j++) {
            m_data[j - diff] = std::move(m_data[j]);
        }
        resize(m_size - diff);
    }

    [[nodiscard]] size_t size() const {
        return m_size;
    }

    [[nodiscard]] size_t capacity() const {
        return m_capacity;
    }

    int const& operator[](size_t i) const {
        return m_data[i];
    }

    int& operator[](size_t i) {
        return m_data[i];
    }

    ~Vectors() { delete[] m_data; }
};
