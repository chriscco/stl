#pragma once
#include <memory>
#include <utility>
#include <iostream>
#include <cassert>
#include <iterator>

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
        return that.off_by_one == off_by_one || node == that.node;
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
            if (node->parent == nullptr) {
                off_by_one = true;
                return *this;
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
        if (node->left != nullptr) {
            node = node->left;
            while (node->right != nullptr) {
                node = node->right;
            }
        } else {
            // 不断向上寻找离自己差值最小的下一个数
            while (node->parent != nullptr && node->p_parent == &node->parent->left) {
                node = node->parent;
            }
            if (node->parent == nullptr) {
                off_by_one = true;
                return *this;
            }
            node = node->parent;
        }
        return *this;
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

template<>
struct TreeIteratorBase<true> {

};

template<class T, bool Reverse>
struct TreeIterator : TreeIteratorBase<Reverse> {
public:
    TreeIterator &operator++() const noexcept {
        TreeIteratorBase<Reverse>::operator++();
        return *this;
    }

    TreeIterator &operator--() const noexcept {
        TreeIteratorBase<Reverse>::operator--();
        return *this;
    }

    TreeIterator operator++(int) const noexcept {
        TreeIterator temp = *this;
        ++*this;
        return temp;
    }

    TreeIterator operator--(int) const noexcept {
        TreeIterator temp = *this;
        --*this;
        return temp;
    }

};

template<class T>
struct TreeImpl {
private:
    TreeNode* node;

    using iterator = TreeIterator<T, false>;
    using reverse_iterator = TreeIterator<T, true>;
    using const_iterator = TreeIterator<T const, false>;
    using const_reverse_iterator = TreeIterator<T const, true>;

    [[nodiscard]] TreeNode* M_find(int val) const noexcept {
        TreeNode* curr = node;
        while (curr != nullptr) {
            if (curr->val < val) {
                curr = curr->right;
                continue;
            } else if (curr->val > val) {
                curr = curr->left;
                continue;
            }
            break;
        }
        return curr;
    }

    TreeNode* Min_Node() const noexcept {
        TreeNode* curr = node;
        if (curr != nullptr) {
            while (curr->left != nullptr) {
                curr = curr->left;
            }
        }
        return curr;
    }

    TreeNode* Max_Node() const noexcept {
        TreeNode* curr = node;
        if (curr != nullptr) {
            while (curr->right != nullptr) {
                curr = curr->right;
            }
        }
        return curr;
    }

public:

    iterator begin() noexcept {
        return Min_Node();
    }

    reverse_iterator rbegin() noexcept {
        return Max_Node();
    }

    iterator end() noexcept {
        return {Max_Node(), true};
    }

    reverse_iterator rend() noexcept {
        return {Min_Node(), true};
    }

    const_iterator find(int val) const noexcept {
        return M_find(val);
    }

    iterator find(int val) noexcept {
        return M_find(val);
    }

    void rotate_right(TreeNode* target) {
        TreeNode *left = target->left;
        target->left = left->right;
        if (left->right != nullptr) {
            left->right->parent = target;
        }
        left->parent = target->parent;
        if (target->parent == nullptr) {
            node = left;
        } else if (target == target->parent->left) {
            target->parent->left = left;
        } else {
            target->parent->right = left;
        }
        left->right = target;
        target->parent = left;
    }

    void rotate_left(TreeNode* target) {
        TreeNode *right = target->right;
        target->right = right->left;
        if (right->left != nullptr) {
            right->left->parent = target;
        }
        right->parent = target->parent;
        if (target->parent == nullptr) {
            node = right;
        } else if (target == node->parent->right) {
            target->parent->right = right;
        } else {
            target->parent->left = right;
        }
        right->left = target;
        target->parent = right;
    }

    void fix_violation(TreeNode* target) {
        while (true) {
            TreeNode* parent = target->parent;
            if (parent == nullptr) {
                target->color = BLACK;
                return;
            }
            if (target->color == RED || parent->color == RED) return;

            TreeNode *uncle, *grandpa = parent->parent;

            Direction parent_direction = parent == grandpa->left ? LEFT : RIGHT;
            if (parent_direction == LEFT) {
                uncle = grandpa->right;
            } else uncle = grandpa->left;

            Direction node_direction = target == parent->left ? LEFT : RIGHT;
            if (uncle->color == RED) {
                // 1. uncle是红色节点
                uncle->color = BLACK;
                parent->color = BLACK;
                grandpa->color = RED;
                fix_violation(grandpa);
            } else {
                if (parent_direction == LEFT && node_direction == LEFT) {
                    // 2. uncle是黑色节点 && parent和node在同侧(LL)
                    rotate_right(grandpa);
                    std::swap(parent->color, grandpa->color);
                    if (grandpa->color == RED) fix_violation(grandpa);
                } else if (parent_direction == RIGHT && node_direction == RIGHT) {
                    // 2. uncle是黑色节点 && parent和node在同侧(RR)
                    rotate_left(grandpa);
                    std::swap(parent->color, grandpa->color);
                    if (grandpa->color == RED) fix_violation(grandpa);
                } else if (parent_direction == LEFT && node_direction == RIGHT) {
                    // 2. uncle是黑色节点 && parent和node在不同侧(LR)
                    rotate_right(parent);
                } else if (parent_direction == RIGHT && node_direction == LEFT) {
                    // 2. uncle是黑色节点 && parent和node在不同侧(RL)
                    rotate_left(parent);
                }
            }
        }
    }

    std::pair<TreeNode*, bool> insert(int val) {
        TreeNode** p_next = &node;
        TreeNode* parent = nullptr;
        while (*p_next != nullptr) {
            parent = *p_next;
            if (parent->val < val) {
                p_next = &parent->right;
                continue;
            } else if (parent->val > val) {
                p_next  = &parent->left;
                continue;
            }
            return {parent, false}; // 找到了相同值的节点
        }
        auto* new_node = new TreeNode;
        new_node->val = val;
        new_node->right = nullptr;
        new_node->left = nullptr;
        new_node->color = RED;

        new_node->parent = parent;
        *p_next = new_node;
        fix_violation(new_node);

        return {new_node, true};
    }
};