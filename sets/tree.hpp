enum Tree_color {
    BLACK,
    RED
};
enum Direction {
    LEFT,
    RIGHT
};

struct TreeNode {
    TreeNode *parent;
    TreeNode* left;
    TreeNode* right;
    Tree_color color;
    int val;
};


