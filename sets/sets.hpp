#pragma once
struct Node {
    Node *parent;
    Node* left;
    Node* right;
    int val;
};

class Sets {
    Node* root = nullptr;

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
};