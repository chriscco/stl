#pragma once

enum Color {
    BLACK,
    RED
};
enum Direction {
    LEFT,
    RIGHT
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

    void rotate_left(Node* node) {
        Node *right = node->right;
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

    void fix_violation(Node* node) {
        while (true) {
            Node* parent = node->parent;
            if (parent == nullptr) {
                node->color = BLACK;
                return;
            }
            if (node->color == RED || parent->color == RED) return;

            Node *uncle, *grandpa = parent->parent;

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

    std::pair<Node*, bool> insert(int val) {
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
            return {parent, false}; // 找到了相同值的节点
        }
        Node* node = new Node;
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