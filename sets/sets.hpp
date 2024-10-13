#pragma once
#include "utils/tree.hpp"

template <class T, class compare = std::less<T>, class allocator = std::allocator<T>>
class Sets {
    TreeNode* root = nullptr;
public:
    struct iterator {
    private:
        TreeNode* node;
        explicit iterator(TreeNode* node) : node(node) {};

    public:
        bool operator==(iterator const& that) const noexcept {
            return node == that.node;
        }

        bool operator!=(iterator const& that) const noexcept {
            return node != that.node;
        }

        iterator &operator++() noexcept {
            return *this;
        }

        iterator &operator--() noexcept {
            return *this;
        }

        iterator operator++(int) noexcept {
            auto tmp = *this;
            ++*this;
            return tmp;
        }

        iterator operator--(int) noexcept {
            auto tmp = *this;
            --*this;
            return tmp;
        }

        friend Sets;
    };
    TreeNode* find(int val) {
        TreeNode* curr = root;
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

    void rotate_right(TreeNode* node) {
        TreeNode *left = node->left;
        node->left = left->right;
        if (left->right != nullptr) {
            left->right->parent = node;
        }
        left->parent = node->parent;
        if (node->parent == nullptr) {
            root = left;
        } else if (node == node->parent->left) {
            node->parent->left = left;
        } else {
            node->parent->right = left;
        }
        left->right = node;
        node->parent = left;
    }

    void rotate_left(TreeNode* node) {
        TreeNode *right = node->right;
        node->right = right->left;
        if (right->left != nullptr) {
            right->left->parent = node;
        }
        right->parent = node->parent;
        if (node->parent == nullptr) {
            root = right;
        } else if (node == node->parent->right) {
            node->parent->right = right;
        } else {
            node->parent->left = right;
        }
        right->left = node;
        node->parent = right;
    }

    void fix_violation(TreeNode* node) {
        while (true) {
            TreeNode* parent = node->parent;
            if (parent == nullptr) {
                node->color = BLACK;
                return;
            }
            if (node->color == RED || parent->color == RED) return;

            TreeNode *uncle, *grandpa = parent->parent;

            Direction parent_direction = parent == grandpa->left ? LEFT : RIGHT;
            if (parent_direction == LEFT) {
                uncle = grandpa->right;
            } else uncle = grandpa->left;

            Direction node_direction = node == parent->left ? LEFT : RIGHT;
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
        TreeNode** p_next = &root;
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
        auto* node = new TreeNode;
        node->val = val;
        node->right = nullptr;
        node->left = nullptr;
        node->color = RED;

        node->parent = parent;
        *p_next = node;
        fix_violation(node);

        return {node, true};
    }
};