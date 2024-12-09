#include <iostream>
#include <vector>
#include <algorithm>
#include <cstring>

const int MIN_DEGREE = 10; // Minimum degree for B-tree
const int MAX_KEYS = 2 * MIN_DEGREE - 1;

struct InMemoryBTreeNode {
    bool isLeaf;
    int numKeys;
    uint64_t keys[MAX_KEYS];
    uint64_t values[MAX_KEYS];
    std::vector<InMemoryBTreeNode *> children;

    InMemoryBTreeNode(bool leaf) : isLeaf(leaf), numKeys(0) {
        std::fill(std::begin(keys), std::end(keys), 0);
        std::fill(std::begin(values), std::end(values), 0);
        children.resize(2 * MIN_DEGREE, nullptr);
    }
};

class InMemoryBTree {
private:
    InMemoryBTreeNode *root;

    void insertNonFull(InMemoryBTreeNode *node, uint64_t key, uint64_t value) {
        int i = node->numKeys - 1;

        if (node->isLeaf) {
            // Find position to insert key
            while (i >= 0 && key < node->keys[i]) {
                node->keys[i + 1] = node->keys[i];
                node->values[i + 1] = node->values[i];
                i--;
            }
            node->keys[i + 1] = key;
            node->values[i + 1] = value;
            node->numKeys++;
        } else {
            // Find child to descend into
            while (i >= 0 && key < node->keys[i]) {
                i--;
            }
            i++;

            if (node->children[i]->numKeys == MAX_KEYS) {
                splitChild(node, i);
                if (key > node->keys[i]) {
                    i++;
                }
            }
            insertNonFull(node->children[i], key, value);
        }
    }

    void splitChild(InMemoryBTreeNode *parent, int index) {
        InMemoryBTreeNode *child = parent->children[index];
        InMemoryBTreeNode *newChild = new InMemoryBTreeNode(child->isLeaf);
        newChild->numKeys = MIN_DEGREE - 1;

        // Move the higher half of the child's keys and values to the new child
        for (int j = 0; j < MIN_DEGREE - 1; j++) {
            newChild->keys[j] = child->keys[j + MIN_DEGREE];
            newChild->values[j] = child->values[j + MIN_DEGREE];
        }

        // Move the higher half of the child's children to the new child
        if (!child->isLeaf) {
            for (int j = 0; j < MIN_DEGREE; j++) {
                newChild->children[j] = child->children[j + MIN_DEGREE];
            }
        }

        child->numKeys = MIN_DEGREE - 1;

        // Insert new child into the parent
        for (int j = parent->numKeys; j >= index + 1; j--) {
            parent->children[j + 1] = parent->children[j];
        }
        parent->children[index + 1] = newChild;

        // Move middle key of the child to the parent
        for (int j = parent->numKeys - 1; j >= index; j--) {
            parent->keys[j + 1] = parent->keys[j];
            parent->values[j + 1] = parent->values[j];
        }
        parent->keys[index] = child->keys[MIN_DEGREE - 1];
        parent->values[index] = child->values[MIN_DEGREE - 1];
        parent->numKeys++;
    }

public:
    InMemoryBTree() : root(new InMemoryBTreeNode(true)) {}

    void insert(uint64_t key, uint64_t value) {
        if (root->numKeys == MAX_KEYS) {
            InMemoryBTreeNode *newRoot = new InMemoryBTreeNode(false);
            newRoot->children[0] = root;
            splitChild(newRoot, 0);
            root = newRoot;
        }
        insertNonFull(root, key, value);
    }

    bool search(uint64_t key) {
        InMemoryBTreeNode *node = root;
        while (node) {
            int i = 0;
            while (i < node->numKeys && key > node->keys[i]) {
                i++;
            }

            if (i < node->numKeys && key == node->keys[i]) {
                std::cout << "Found key " << key << " with value " << node->values[i] << "\n";
                return true;
            }

            if (node->isLeaf) {
                return false;
            }

            node = node->children[i];
        }
        return false;
    }

    void printTree() {
        printTree(root, 0);
    }

    void printTree(InMemoryBTreeNode *node, int depth) {
        if (!node) return;
        std::cout << std::string(depth * 2, ' ') << "Node with " << node->numKeys << " keys: ";
        for (int i = 0; i < node->numKeys; i++) {
            std::cout << node->keys[i] << " ";
        }
        std::cout << "\n";
        for (int i = 0; i <= node->numKeys; i++) {
            printTree(node->children[i], depth + 1);
        }
    }
};

int main() {
    InMemoryBTree btree;

    btree.insert(10, 100);
    btree.insert(20, 200);
    btree.insert(5, 50);
    btree.insert(6, 60);
    btree.insert(12, 120);

    btree.printTree();

    if (btree.search(20)) {
        std::cout << "Search successful.\n";
    } else {
        std::cout << "Key not found.\n";
    }

    if (btree.search(15)) {
        std::cout << "Search successful.\n";
    } else {
        std::cout << "Key not found.\n";
    }

    return 0;
}
