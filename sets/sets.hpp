#pragma once
struct Node {
    Node *parent;
    Node* left;
    Node* right;
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

    bool insert(int val) {
        Node* node = new Node;
        node->val = val;
        node->right = nullptr;
        node->left = nullptr;

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

        return true;
    }
};