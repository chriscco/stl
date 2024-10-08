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
        resize(size() + 1);
        back() = val;
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
