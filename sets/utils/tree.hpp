#pragma once
#include <memory>
#include <utility>
#include <iostream>
#include <cassert>

enum Tree_color {
    BLACK,
    RED
};
enum Direction {
    LEFT,
    RIGHT
};

struct TreeNode {
    TreeNode* left;
    TreeNode* right;
    TreeNode *parent;
    TreeNode** p_parent; // 父节点中指向本节点的指针
    Tree_color color;

    int val;
};

class TreeNodeImpl : public TreeNode {

};

template <bool>
struct TreeIteratorBase;

template<>
struct TreeIteratorBase<false> {
protected:
    TreeNode* node;
    bool off_by_one;

public:
    TreeIteratorBase(bool offByOne, TreeNode *node) : off_by_one(offByOne), node(node) {}
    explicit TreeIteratorBase() : node(nullptr), off_by_one(false) {}

    bool operator==(TreeIteratorBase const& that) const noexcept {
        return that.off_by_one == off_by_one && node == that.node;
    }
    bool operator!=(TreeIteratorBase const& that) const noexcept {
        return this != &that;
    }
    TreeIteratorBase &operator++() noexcept {
        assert(!off_by_one);
        assert(node);
        if (node->right != nullptr) {
            node = node->right;
            while (node->left != nullptr) {
                node = node->left;
            }
        } else {
            // 不断向上寻找离自己差值最小的下一个数
            while (node->parent != nullptr && node->p_parent == &node->parent->right) {
                node = node->parent;
            }
            node = node->parent;
        }
        return *this;
    }
    TreeIteratorBase &operator--() noexcept {
        if (off_by_one) {
            off_by_one = false;
            return *this;
        }
    }
    TreeIteratorBase operator++(int) noexcept {
        auto tmp = *this;
        ++*this;
        return tmp;
    }
    TreeIteratorBase operator--(int) noexcept {
        auto tmp = *this;
        --*this;
        return tmp;
    }
    
};


