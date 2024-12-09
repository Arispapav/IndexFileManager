#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <cstring>

const int MIN_DEGREE = 10; // Minimum degree for B-tree
const int MAX_KEYS = 2 * MIN_DEGREE - 1; // Maximum keys in a node
const int MAX_CHILDREN = 2 * MIN_DEGREE; // Maximum children in a node

struct BTreeNode {
    bool isLeaf;
    int numKeys;
    uint64_t keys[MAX_KEYS];
    uint64_t values[MAX_KEYS];
    uint64_t children[MAX_CHILDREN];

    // Constructor to initialize a node
    BTreeNode(bool leaf = true) {
        isLeaf = leaf;
        numKeys = 0;
        std::fill(std::begin(keys), std::end(keys), 0);
        std::fill(std::begin(values), std::end(values), 0);
        std::fill(std::begin(children), std::end(children), 0);
    }
};

class BTree {
private:
    std::fstream file;
    uint64_t rootBlockId;
    uint64_t nextBlockId;

    // Helper function to load a node from the file
    BTreeNode loadNode(uint64_t blockId) {
        file.seekg(blockId * 512, std::ios::beg);
        uint8_t buffer[512] = {0};
        file.read(reinterpret_cast<char *>(buffer), 512);

        BTreeNode node;
        std::memcpy(&node, buffer, sizeof(BTreeNode));
        return node;
    }

    // Helper function to save a node to the file
    void saveNode(uint64_t blockId, const BTreeNode &node) {
        file.seekp(blockId * 512, std::ios::beg);
        uint8_t buffer[512] = {0};
        std::memcpy(buffer, &node, sizeof(BTreeNode));
        file.write(reinterpret_cast<const char *>(buffer), 512);
    }

    // Recursive helper function for insertion
    void insertNonFull(BTreeNode node, uint64_t key, uint64_t value, uint64_t blockId) {
    int i = node.numKeys - 1;

    if (node.isLeaf) {
        // Find location to insert
        while (i >= 0 && key < node.keys[i]) {
            node.keys[i + 1] = node.keys[i];
            node.values[i + 1] = node.values[i];
            i--;
        }
        node.keys[i + 1] = key;
        node.values[i + 1] = value;
        node.numKeys++;
        saveNode(blockId, node);
    } else {
        while (i >= 0 && key < node.keys[i]) {
            i--;
        }
        i++;

        uint64_t childBlockId = node.children[i];
        BTreeNode childNode = loadNode(childBlockId);

        if (childNode.numKeys == MAX_KEYS) {
            splitChild(node, i, childBlockId);
            if (key > node.keys[i]) {
                i++;
            }
        }
        // Pass childNode explicitly
        insertNonFull(childNode, key, value, node.children[i]);
    }
}


    // Split a full child node
    void splitChild(BTreeNode &parent, int index, uint64_t childBlockId) {
        BTreeNode childNode = loadNode(childBlockId);
        BTreeNode newChildNode(childNode.isLeaf);
        newChildNode.numKeys = MIN_DEGREE - 1;

        // Move the higher half of the keys and values to the new child
        for (int j = 0; j < MIN_DEGREE - 1; j++) {
            newChildNode.keys[j] = childNode.keys[j + MIN_DEGREE];
            newChildNode.values[j] = childNode.values[j + MIN_DEGREE];
        }

        if (!childNode.isLeaf) {
            for (int j = 0; j < MIN_DEGREE; j++) {
                newChildNode.children[j] = childNode.children[j + MIN_DEGREE];
            }
        }

        childNode.numKeys = MIN_DEGREE - 1;
        saveNode(childBlockId, childNode);

        uint64_t newChildBlockId = nextBlockId++;
        saveNode(newChildBlockId, newChildNode);

        // Shift children of parent to make space
        for (int j = parent.numKeys; j >= index + 1; j--) {
            parent.children[j + 1] = parent.children[j];
        }

        parent.children[index + 1] = newChildBlockId;

        // Shift keys and values of parent
        for (int j = parent.numKeys - 1; j >= index; j--) {
            parent.keys[j + 1] = parent.keys[j];
            parent.values[j + 1] = parent.values[j];
        }

        parent.keys[index] = childNode.keys[MIN_DEGREE - 1];
        parent.values[index] = childNode.values[MIN_DEGREE - 1];
        parent.numKeys++;
        saveNode(rootBlockId, parent);
    }

public:
    BTree(const std::string &fileName) {
        file.open(fileName, std::ios::in | std::ios::out | std::ios::binary);
        if (!file.is_open()) {
            file.open(fileName, std::ios::out | std::ios::binary);
            file.close();
            file.open(fileName, std::ios::in | std::ios::out | std::ios::binary);

            rootBlockId = 0;
            nextBlockId = 1;

            BTreeNode root;
            saveNode(rootBlockId, root);
        } else {
            // Load header to initialize root and nextBlockId
            uint8_t header[512] = {0};
            file.read(reinterpret_cast<char *>(header), 512);
            std::memcpy(&rootBlockId, header + 8, sizeof(rootBlockId));
            std::memcpy(&nextBlockId, header + 16, sizeof(nextBlockId));
        }
    }

   void insert(uint64_t key, uint64_t value) {
    BTreeNode root = loadNode(rootBlockId);

    if (root.numKeys == MAX_KEYS) {
        BTreeNode newRoot(false);
        newRoot.children[0] = rootBlockId;
        splitChild(newRoot, 0, rootBlockId);

        rootBlockId = nextBlockId++;
        saveNode(rootBlockId, newRoot);
    }

    insertNonFull(loadNode(rootBlockId), key, value, rootBlockId);
}


    bool search(uint64_t key) {
        BTreeNode node = loadNode(rootBlockId);

        while (true) {
            int i = 0;
            while (i < node.numKeys && key > node.keys[i]) {
                i++;
            }

            if (i < node.numKeys && key == node.keys[i]) {
                std::cout << "Found key " << key << " with value " << node.values[i] << "\n";
                return true;
            }

            if (node.isLeaf) {
                return false;
            }

            node = loadNode(node.children[i]);
        }
    }
};


