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
        m_data = new int[size]{};
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
        grow_capacity_until(size);
        m_size = size;
    }

    void grow_capacity_until(size_t n) {
        if (n <= m_capacity) [[likely]] return;
        n = std::max(n, m_capacity * 2);
        printf("grow from %zd to %zd\n", m_capacity, n);
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

    [[nodiscard]] size_t size() const {
        return m_size;
    }

    int const& operator[](size_t i) const {
        return m_data[i];
    }

    int& operator[](size_t i) {
        return m_data[i];
    }

    ~Vectors() { delete[] m_data; }
};
