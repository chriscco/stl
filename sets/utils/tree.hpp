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
struct TreeIterator : protected TreeIteratorBase<Reverse> {
protected:
    TreeIterator(TreeNode* node, bool off_by_one) : TreeIteratorBase<Reverse>(node, off_by_one) {};
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

    T *operator->() const noexcept {
        return &this->node->val;
    }

    T operator*() const noexcept {
        return this->node->val;
    }

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

    [[nodiscard]] TreeNode* Min_Node() const noexcept {
        TreeNode* curr = node;
        if (curr != nullptr) {
            while (curr->left != nullptr) {
                curr = curr->left;
            }
        }
        return curr;
    }

    [[nodiscard]] TreeNode* Max_Node() const noexcept {
        TreeNode* curr = node;
        if (curr != nullptr) {
            while (curr->right != nullptr) {
                curr = curr->right;
            }
        }
        return curr;
    }

    static void M_rotate_right(TreeNode* target) noexcept {
        TreeNode *left = target->left;
        target->right = left->right;
        if (left->right != nullptr) {
            left->right->parent = target;
            left->right->p_parent = &target->left;
        }
        left->parent = target->parent;
        left->p_parent = target->p_parent;
        *target->p_parent = left;
        left->right = target;
        target->parent = left;
        target->p_parent = &left->right;
    }

    static void M_rotate_left(TreeNode* target) noexcept {
        // 获取 target 的右子节点
        TreeNode *right = target->right;

        // 将 target 的右子节点的左子节点连接到 target 的右子节点
        target->right = right->left;

        // 如果 right 的左子节点不为空，更新其父节点和 p_parent
        if (right->left != nullptr) {
            right->left->parent = target; // 设置左子节点的父节点为 target
            right->left->p_parent = &target->right; // 设置左子节点的 p_parent
        }

        // 更新 right 的父节点和 p_parent
        right->parent = target->parent; // 将 right 的父节点设为 target 的父节点
        right->p_parent = target->p_parent; // 更新 p_parent

        // 更新 target 的 p_parent 的指向
        *target->p_parent = right; // 将 target 的 p_parent 指向 right

        // 将 target 左旋转，使其成为 right 的左子节点
        right->left = target; // 将 target 设为 right 的左子节点
        target->parent = right; // 更新 target 的父节点为 right
        target->p_parent = &right->left; // 更新 target 的 p_parent
    }

    static void M_fix_violation(TreeNode* target) noexcept {
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
                M_fix_violation(grandpa);
            } else {
                if (parent_direction == LEFT && node_direction == LEFT) {
                    // 2. uncle是黑色节点 && parent和node在同侧(LL)
                    M_rotate_right(grandpa);
                    std::swap(parent->color, grandpa->color);
                    if (grandpa->color == RED) M_fix_violation(grandpa);
                } else if (parent_direction == RIGHT && node_direction == RIGHT) {
                    // 2. uncle是黑色节点 && parent和node在同侧(RR)
                    M_rotate_left(grandpa);
                    std::swap(parent->color, grandpa->color);
                    if (grandpa->color == RED) M_fix_violation(grandpa);
                } else if (parent_direction == LEFT && node_direction == RIGHT) {
                    // 2. uncle是黑色节点 && parent和node在不同侧(LR)
                    M_rotate_right(parent);
                } else if (parent_direction == RIGHT && node_direction == LEFT) {
                    // 2. uncle是黑色节点 && parent和node在不同侧(RL)
                    M_rotate_left(parent);
                }
            }
        }
    }

    std::pair<TreeNode*, bool> M_insert(int val) {
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
        M_fix_violation(new_node);

        return {new_node, true};
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

    std::pair<TreeNode*, bool> insert(int val) {
        return M_insert(val);
    }

    size_t count(int val) const noexcept {
        return find(val) == end() ? 0 : 1;
    }

    size_t contains(int val) const noexcept {
        return find(val) != end();
    }
};