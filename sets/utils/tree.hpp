#pragma once

#include <memory>
#include <utility>
#include <iostream>
#include <cassert>
#include <iterator>

enum RBTree_color {
    BLACK,
    RED
};
enum RBDirection {
    LEFT,
    RIGHT
};

struct RBTreeNode {
    RBTreeNode* left;
    RBTreeNode* right;
    RBTreeNode *parent;
    RBTreeNode** p_parent; // 父节点中指向本节点的指针
    RBTree_color color;

    int val;
};

template <bool>
struct RBTreeIteratorBase;

template<>
struct RBTreeIteratorBase<false> {
    union {
        RBTreeNode* node;
        RBTreeNode** p_root;
    };
    bool off_by_one;

public:

    template<class, bool>
    friend struct RBTreeIterator;

    RBTreeIteratorBase(bool offByOne, RBTreeNode *node) : off_by_one(offByOne), node(node) {}
    explicit RBTreeIteratorBase() : node(nullptr), off_by_one(false) {}

    bool operator==(RBTreeIteratorBase const& that) const noexcept {
        return that.off_by_one == off_by_one || node == that.node;
    }
    bool operator!=(RBTreeIteratorBase const& that) const noexcept {
        return this != &that;
    }
    void operator++() noexcept {
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
                return;
            }
            node = node->parent;
        }
    }
    void operator--() noexcept {
        if (off_by_one) {
            off_by_one = false;
            return;
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
                return;
            }
            node = node->parent;
        }
    }
};

template<>
struct RBTreeIteratorBase<true> : RBTreeIteratorBase<false> {
protected:
    using RBTreeIteratorBase<false>::RBTreeIteratorBase;
};

template<class T, bool Reverse>
struct RBTreeIterator : protected RBTreeIteratorBase<Reverse> {
protected:
    using RBTreeIteratorBase<Reverse>::RBTreeIteratorBase;

    RBTreeIterator(RBTreeNode* node, bool off_by_one) : RBTreeIteratorBase<Reverse>(node, off_by_one) {};
public:
    template<class T0 = T>
    explicit operator std::enable_if_t<std::is_const_v<T0>, RBTreeIterator<std::remove_const_t<T0>, Reverse>>()
    const noexcept {
        if (!this->off_by_one) {
            return this->node;
        } else {
            return this->p_root;
        }
    }

    template<class T0 = T>
    explicit operator std::enable_if_t<!std::is_const_v<T0>, RBTreeIterator<std::add_const_t<T0>, Reverse>>()
    const noexcept {
        if (!this->off_by_one) {
            return this->node;
        } else {
            return this->p_root;
        }
    }
    RBTreeIterator &operator++() const noexcept {
        RBTreeIteratorBase<Reverse>::operator++();
        return *this;
    }

    RBTreeIterator &operator--() const noexcept {
        RBTreeIteratorBase<Reverse>::operator--();
        return *this;
    }

    RBTreeIterator operator++(int) const noexcept {
        RBTreeIterator temp = *this;
        ++*this;
        return temp;
    }

    RBTreeIterator operator--(int) const noexcept {
        RBTreeIterator temp = *this;
        --*this;
        return temp;
    }
};

struct TreeRoot {
    RBTreeNode* m_node;
    TreeRoot() noexcept : m_node(nullptr) {};
};

template<class T>
struct TreeBase {
protected:
    TreeRoot *m_block;
public:
    using iterator = RBTreeIterator<T, false>;
    using reverse_iterator = RBTreeIterator<T, true>;
    using const_iterator = RBTreeIterator<T const, false>;
    using const_reverse_iterator = RBTreeIterator<T const, true>;

    TreeBase() noexcept : m_block(new TreeRoot) {};

    TreeBase(TreeBase &&that) noexcept : m_block(that.m_block) {
        that.m_block = nullptr;
    }

    TreeBase& operator=(TreeBase &&that) noexcept {
        std::swap(that.m_block, m_block);
        return *this;
    }

protected:
    [[nodiscard]] RBTreeNode* M_find(int val) const noexcept {
        RBTreeNode* curr = m_block->m_node;
        while (curr != nullptr) {
            if (curr->val < val) {
                curr = curr->right;
                continue;
            } else if (curr->val > val) {
                curr = curr->left;
                continue;
            }
            return curr;
        }
        return nullptr;
    }

    [[nodiscard]] RBTreeNode* Min_Node() const noexcept {
        RBTreeNode* curr = m_block->m_node;
        if (curr != nullptr) {
            while (curr->left != nullptr) {
                curr = curr->left;
            }
        }
        return curr;
    }

    [[nodiscard]] RBTreeNode* Max_Node() const noexcept {
        RBTreeNode* curr = m_block->m_node;
        if (curr != nullptr) {
            while (curr->right != nullptr) {
                curr = curr->right;
            }
        }
        return curr;
    }

    static void M_rotate_right(RBTreeNode* target) noexcept {
        RBTreeNode *left = target->left;
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

    static void M_rotate_left(RBTreeNode* target) noexcept {
        // 获取 target 的右子节点
        RBTreeNode *right = target->right;

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

    static void M_fix_violation(RBTreeNode* target) noexcept {
        while (true) {
            RBTreeNode* parent = target->parent;
            if (parent == nullptr) {
                target->color = BLACK;
                return;
            }
            if (target->color == RED || parent->color == RED) return;

            RBTreeNode *uncle, *grandpa = parent->parent;

            RBDirection parent_direction = parent == grandpa->left ? LEFT : RIGHT;
            if (parent_direction == LEFT) {
                uncle = grandpa->right;
            } else uncle = grandpa->left;

            RBDirection node_direction = target == parent->left ? LEFT : RIGHT;
            if (uncle->color == RED) {
                // 1. uncle是红色节点
                uncle->color = BLACK;
                parent->color = BLACK;
                grandpa->color = RED;
                TreeBase::M_fix_violation(grandpa);
            } else {
                if (parent_direction == LEFT && node_direction == LEFT) {
                    // 2. uncle是黑色节点 && parent和node在同侧(LL)
                    TreeBase::M_rotate_right(grandpa);
                    std::swap(parent->color, grandpa->color);
                    if (grandpa->color == RED) M_fix_violation(grandpa);
                } else if (parent_direction == RIGHT && node_direction == RIGHT) {
                    // 2. uncle是黑色节点 && parent和node在同侧(RR)
                    TreeBase::M_rotate_left(grandpa);
                    std::swap(parent->color, grandpa->color);
                    if (grandpa->color == RED) M_fix_violation(grandpa);
                } else if (parent_direction == LEFT && node_direction == RIGHT) {
                    // 2. uncle是黑色节点 && parent和node在不同侧(LR)
                    TreeBase::M_rotate_right(parent);
                } else if (parent_direction == RIGHT && node_direction == LEFT) {
                    // 2. uncle是黑色节点 && parent和node在不同侧(RL)
                    TreeBase::M_rotate_left(parent);
                }
            }
        }
    }

    RBTreeNode* M_single_insert(int val) {
        RBTreeNode** p_parent = &m_block->m_node;
        RBTreeNode* parent = nullptr;
        while (*p_parent != nullptr) {
            parent = *p_parent;
            if (parent->val < val) {
                p_parent = &parent->right;
                continue;
            } else if (parent->val > val) {
                p_parent  = &parent->left;
                continue;
            }
            return parent; // 找到了相同值的节点
        }
        auto* new_node = new RBTreeNode;
        new_node->val = val;
        new_node->right = nullptr;
        new_node->left = nullptr;
        new_node->color = RED;

        new_node->parent = parent;
        new_node->p_parent = p_parent;
        *p_parent = new_node;
        TreeBase::M_fix_violation(new_node);

        return nullptr;
    }

    iterator M_multi_insert(int val) {
        RBTreeNode** p_parent = &m_block->m_node;
        RBTreeNode* parent = nullptr;
        while (*p_parent != nullptr) {
            parent = *p_parent;
            if (parent->val < val) {
                p_parent = &parent->right;
                continue;
            } else if (parent->val > val) {
                p_parent  = &parent->left;
                continue;
            }
        }
        auto* new_node = new RBTreeNode;
        new_node->val = val;
        new_node->right = nullptr;
        new_node->left = nullptr;
        new_node->color = RED;

        new_node->parent = parent;
        new_node->p_parent = p_parent;
        *p_parent = new_node;
        TreeBase::M_fix_violation(new_node);

        return new_node;
    }

public:

    T *operator->() const noexcept {
        return &this->m_block->m_node->val;
    }

    T operator*() const noexcept {
        return this->m_block->m_node->val;
    }


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

    iterator _M_find(int val) noexcept {
        RBTreeNode *res = M_find(val);
        if (res) return m_block->m_node;
        else return end();
    }

    const_iterator _M_find(int val) const noexcept {
        RBTreeNode *res = M_find(val);
        if (res) return m_block->m_node;
        else return end();
    }

    std::pair<iterator, bool> single_insert(int val) {
        return {M_single_insert(val), true};
    }

    iterator multi_insert(int val) {
        return M_multi_insert(val);
    }

    size_t count(int val) const noexcept {
        return M_find(val) != nullptr ? 0 : 1;
    }

    size_t contains(int val) const noexcept {
        return M_find(val) != nullptr;
    }

};

template<class T>
struct TreeImpl : protected TreeBase<T> {

};