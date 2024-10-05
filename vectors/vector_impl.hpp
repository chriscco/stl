#include <vector>
#include <cstring>
#ifndef STL_VECTORS_HPP
#define STL_VECTORS_HPP

class Vectors {
private:
    int* m_data;
    size_t m_size;

public:
    Vectors() : m_data(nullptr), m_size(0) {};

    explicit Vectors(size_t size) {
        m_data = new int[size]{};
        m_size = size;
    }

    /**
     * 深拷贝防止析构m_data两次
     * @param that
     */
    Vectors(Vectors const& that) {
        m_size = that.m_size;
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

    void clear() {
        resize(0);
    }

    void resize(size_t size) {
        auto old_data = m_data;
        auto old_size = m_size;

        if (size == 0) {
            m_data = nullptr;
            m_size = 0;
        } else {
            m_data = new int[size]{};
            m_size = size;
        }
        if (old_data) {
            size_t copy_size = std::min(size, old_size);
            if (copy_size != 0) {
                memcpy(m_data, old_data, std::min(size, old_size) * sizeof(int));
            }

            delete[] old_data;
        }

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


#endif //STL_VECTORS_HPP
