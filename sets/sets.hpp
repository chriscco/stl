#pragma once

enum Color {
    BLACK,
    RED
};

struct Node {
    Node *parent;
    Node* left;
    Node* right;
    Color color;

    int val;
};

class Sets {
    Node* root = nullptr;
public:
    Node* find(int val) {
        Node* curr = root;
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

    void rotate_right(Node* node) {
        Node *left = node->left;
        while (node->parent != nullptr) {
            if (node == node->parent->left) {
                node->parent->left = left;
            } else {
                node->parent->right = left;
            }
        }
        left->right = node;
        node->left = left->right;
    }

    void rotate_left(Node* node) {
        Node *right = node->right;
        while (node->parent != nullptr) {
            if (node == node->parent->left) {
                node->parent->left = right;
            } else {
                node->parent->right = right;
            }
        }
        right->left = node;
        node->right = right->left;
    }

    void fix_violation(Node* node) {
        if (node->parent == nullptr) {
            node->color = BLACK;
            return;
        }
        while (node->color == RED && )
    }

    bool insert(int val) {
        Node* node = new Node;
        node->val = val;
        node->right = nullptr;
        node->left = nullptr;
        node->color = RED;

        Node** p_next = &root;
        Node* parent = nullptr;
        while (*p_next != nullptr) {
            parent = *p_next;
            if (parent->val < val) {
                p_next = &parent->right;
                continue;
            } else if (parent->val > val) {
                p_next  = &parent->left;
                continue;
            }
            return false; // 找到了相同值的节点
        }
        node->parent = parent;
        *p_next = node;
        fix_violation(node);

        return true;
    }
};