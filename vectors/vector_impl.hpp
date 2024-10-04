#include <vector>
#ifndef STL_VECTORS_HPP
#define STL_VECTORS_HPP

class Vectors {
private:
    int* m_data;
    size_t m_size;

public:
    explicit Vectors(size_t size) {
        m_data = new int[size];
        m_size = size;
    }
    size_t size() const {
        return m_size;
    }

    int operator[](size_t i) {
        return m_data[i];
    }

    ~Vectors() { delete m_data; }
};


#endif //STL_VECTORS_HPP
