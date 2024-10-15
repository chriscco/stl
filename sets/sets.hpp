#pragma once
#include "utils/tree.hpp"

template <class T, class compare = std::less<T>, class allocator = std::allocator<T>>
class Sets : TreeBase<T> {
private:
    using typename TreeBase<T>::const_iterator;
    using iterator = const_iterator;
    // using typename TreeBase<T>::const_iterator;
public:
    Sets() = default;

    std::pair<iterator, bool> insert(T val) {
        return TreeBase<T>::single_insert(val);
    }

    iterator find(T val) {
        return this->_M_find(val);
    }

    const_iterator find(T val) const noexcept {
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