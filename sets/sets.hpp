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
                curr = curr->left;
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

        Node** parent = &root;
        while (*parent != nullptr) {
            if ((*parent)->val < val) {
                parent = &(*parent)->right;
                continue;
            } else if ((*parent)->val > val) {
                parent  = &(*parent)->left;
                continue;
            }
            return false; // 找到了相同值的节点
        }
        *parent = node;

        return true;
    }
};