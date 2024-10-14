#pragma once
#include "utils/tree.hpp"

template <class T, class compare = std::less<T>, class allocator = std::allocator<T>>
class Sets : protected TreeBase<T> {
private:
    using iterator = typename TreeBase<T>::iterator;
    using const_iterator = typename TreeBase<T>::const_iterator;
public:
    Sets() = default;

    std::pair<iterator, bool> insert(int val) {
        return this->single_insert(val);
    }

    iterator find(int val) {
        return this->_M_find(val);
    }

    const_iterator find(int val) const noexcept {
        return this->_M_find(val);
    }

};

template <class T, class compare = std::less<T>, class allocator = std::allocator<T>>
class MultiSet : TreeBase<T> {
private:
    using iterator = typename TreeBase<T>::iterator;
public:
    iterator insert(int val) {
        return this->multi_insert(val);
    }
};