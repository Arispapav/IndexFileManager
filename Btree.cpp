#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cstring>
#include <vector>
#include <algorithm>

using namespace std;

// Constants and parameters for the B-Tree
static const string MAGIC_NUMBER = "4337PRJ3";
static const int HEADER_SIZE = 512;
static const size_t BLOCK_SIZE = 512;
static const int MIN_DEGREE = 10;
static const int MAX_KEYS = (2 * MIN_DEGREE - 1);  // Maximum number of keys in a node: 19
static const int MAX_CHILDREN = (2 * MIN_DEGREE);  // Maximum number of children in a node: 20

// Forward declarations of big-endian functions (implemented elsewhere)
uint64_t hostToBig(uint64_t x);
uint64_t bigToHost(uint64_t x);

// B-Tree node structure stored on disk
struct BTreeNode {
    uint64_t blockId;                  // The block ID where this node is stored
    uint64_t parentId;                 // The block ID of this node's parent, 0 if root
    uint64_t numKeys;                  // Number of keys currently in this node
    uint64_t keys[MAX_KEYS];           // Array of keys
    uint64_t values[MAX_KEYS];         // Array of values corresponding to the keys
    uint64_t children[MAX_CHILDREN];   // Array of child block IDs
    bool isLeaf;                       // Flag indicating if the node is a leaf (no children)

    BTreeNode() {
        memset(this, 0, sizeof(BTreeNode));
    }
};

class BTree {
private:
    fstream file;         // File stream for reading/writing the index file
    string fileName;      // Name of the currently opened file
    uint64_t rootBlockId; // Block ID of the root node
    uint64_t nextBlockId; // Next available block ID for new nodes
    bool fileOpen;        // Flag indicating if a file is currently open

    // Write the B-Tree header into the file (contains magic number, root ID, next block ID)
    void writeHeader() {
        file.seekp(0, ios::beg);
        char header[HEADER_SIZE];
        memset(header, 0, HEADER_SIZE);

        // Copy magic number to header
        memcpy(header, MAGIC_NUMBER.c_str(), MAGIC_NUMBER.size());

        // Convert and store rootBlockId and nextBlockId in big-endian
        uint64_t beRoot = hostToBig(rootBlockId);
        uint64_t beNext = hostToBig(nextBlockId);
        memcpy(header+8, &beRoot, sizeof(beRoot));
        memcpy(header+16, &beNext, sizeof(beNext));

        // Write header to file
        file.write(header, HEADER_SIZE);
        file.flush();
    }

    // Read and validate the B-Tree header from the file
    void readHeader() {
        file.seekg(0, ios::beg);
        char header[HEADER_SIZE] = {0};
        file.read(header, HEADER_SIZE);

        // Check if the header is correctly sized
        if ((size_t)file.gcount() < HEADER_SIZE) {
            throw runtime_error("Invalid file header.");
        }

        // Validate the magic number
        if (strncmp(header, MAGIC_NUMBER.c_str(), MAGIC_NUMBER.size()) != 0) {
            throw runtime_error("Magic number mismatch.");
        }

        // Extract and convert rootBlockId and nextBlockId from header
        uint64_t beRoot = 0;
        uint64_t beNext = 0;
        memcpy(&beRoot, header+8, sizeof(beRoot));
        memcpy(&beNext, header+16, sizeof(beNext));
        rootBlockId = bigToHost(beRoot);
        nextBlockId = bigToHost(beNext);
    }

    // Load a node from the file at a given block ID
    BTreeNode loadNode(uint64_t blockId) {
        file.seekg(blockId * BLOCK_SIZE, ios::beg);
        uint8_t buffer[BLOCK_SIZE];
        memset(buffer, 0, BLOCK_SIZE);
        file.read((char*)buffer, BLOCK_SIZE);

        BTreeNode node;

        // Extract blockId, parentId, and numKeys from the node block
        uint64_t beBlockId, beParentId, beNumKeys;
        memcpy(&beBlockId, buffer, 8);
        memcpy(&beParentId, buffer+8, 8);
        memcpy(&beNumKeys, buffer+16, 8);

        node.blockId = bigToHost(beBlockId);
        node.parentId = bigToHost(beParentId);
        node.numKeys = bigToHost(beNumKeys);

        // Extract keys
        for (int i=0; i<MAX_KEYS; i++) {
            uint64_t beKey;
            memcpy(&beKey, buffer+24+(i*8), 8);
            node.keys[i] = bigToHost(beKey);
        }

        // Extract values
        for (int i=0; i<MAX_KEYS; i++) {
            uint64_t beVal;
            memcpy(&beVal, buffer+24+(MAX_KEYS*8)+(i*8), 8);
            node.values[i] = bigToHost(beVal);
        }

        // Extract children block IDs
        for (int i=0; i<MAX_CHILDREN; i++) {
            uint64_t beChild;
            memcpy(&beChild, buffer+24+(MAX_KEYS*8)+(MAX_KEYS*8)+(i*8), 8);
            node.children[i] = bigToHost(beChild);
        }

        // Determine if the node is a leaf (no children)
        bool leaf = true;
        for (int i=0; i<MAX_CHILDREN; i++) {
            if (node.children[i] != 0) {
                leaf = false;
                break;
            }
        }
        node.isLeaf = leaf;

        return node;
    }

    // Save a node's data to the file at its block ID
    void saveNode(const BTreeNode &node) {
        file.seekp(node.blockId * BLOCK_SIZE, ios::beg);
        uint8_t buffer[BLOCK_SIZE];
        memset(buffer, 0, BLOCK_SIZE);

        // Convert fields to big-endian before writing
        uint64_t beBlockId = hostToBig(node.blockId);
        uint64_t beParentId = hostToBig(node.parentId);
        uint64_t beNumKeys = hostToBig(node.numKeys);

        memcpy(buffer, &beBlockId, 8);
        memcpy(buffer+8, &beParentId, 8);
        memcpy(buffer+16, &beNumKeys, 8);

        // Store keys
        for (int i=0; i<MAX_KEYS; i++) {
            uint64_t beKey = hostToBig(node.keys[i]);
            memcpy(buffer+24+(i*8), &beKey, 8);
        }

        // Store values
        for (int i=0; i<MAX_KEYS; i++) {
            uint64_t beVal = hostToBig(node.values[i]);
            memcpy(buffer+24+(MAX_KEYS*8)+(i*8), &beVal, 8);
        }

        // Store children
        for (int i=0; i<MAX_CHILDREN; i++) {
            uint64_t beChild = hostToBig(node.children[i]);
            memcpy(buffer+24+(MAX_KEYS*8)+(MAX_KEYS*8)+(i*8), &beChild, 8);
        }

        // Write node block to file
        file.write((char*)buffer, BLOCK_SIZE);
        file.flush();
    }

    // Allocate a new node on disk, updating nextBlockId and header
    BTreeNode allocateNode(bool leaf) {
        BTreeNode node;
        node.blockId = nextBlockId++;
        node.parentId = 0;
        node.numKeys = 0;
        saveNode(node);
        writeHeader();
        return node;
    }

    // Search for a key in the B-Tree, return true if found and set valueOut
    bool searchKey(uint64_t blockId, uint64_t key, uint64_t &valueOut) {
        if (blockId == 0) return false; // No such node
        BTreeNode node = loadNode(blockId);

        int i = 0;
        // Find the position of the key or where it would be inserted
        while (i < (int)node.numKeys && key > node.keys[i]) i++;

        // If key is found in this node, return its value
        if (i < (int)node.numKeys && key == node.keys[i]) {
            valueOut = node.values[i];
            return true;
        }

        // If leaf, key not found
        if (node.isLeaf) {
            return false;
        }

        // Otherwise, search in the appropriate child
        return searchKey(node.children[i], key, valueOut);
    }

    // Check if a key already exists in the B-Tree
    bool keyExists(uint64_t key) {
        if (rootBlockId == 0) return false; // Empty tree
        uint64_t dummy;
        return searchKey(rootBlockId, key, dummy);
    }

    // Insert a key/value pair into the B-Tree
    void insertKey(uint64_t key, uint64_t value) {
        if (rootBlockId == 0) {
            // Tree is empty, create a new root node
            BTreeNode root = allocateNode(true);
            root.numKeys = 1;
            root.keys[0] = key;
            root.values[0] = value;
            saveNode(root);
            rootBlockId = root.blockId;
            writeHeader();
            return;
        }

        // If root is full, split it before inserting
        BTreeNode root = loadNode(rootBlockId);
        if (root.numKeys == MAX_KEYS) {
            BTreeNode newRoot = allocateNode(false);
            newRoot.children[0] = root.blockId;
            root.parentId = newRoot.blockId;
            saveNode(root);

            // Split the old root and create a new root
            splitChild(newRoot, 0, root.blockId);
            saveNode(newRoot);
            rootBlockId = newRoot.blockId;
            writeHeader();

            insertNonFull(newRoot, key, value);
        } else {
            // Root not full, insert into it
            insertNonFull(root, key, value);
        }
    }

    // Insert into a node that is guaranteed not to be full
    void insertNonFull(BTreeNode node, uint64_t key, uint64_t value) {
        while (true) {
            int i = (int)node.numKeys - 1;
            if (node.isLeaf) {
                // Insert key/value into leaf node
                while (i>=0 && key < node.keys[i]) {
                    node.keys[i+1] = node.keys[i];
                    node.values[i+1] = node.values[i];
                    i--;
                }
                // Check for duplicate key
                if (i>=0 && node.keys[i] == key) {
                    // Key already exists, abort
                    saveNode(node);
                    throw runtime_error("Key already exists.");
                }
                node.keys[i+1] = key;
                node.values[i+1] = value;
                node.numKeys++;
                saveNode(node);
                return;
            } else {
                // Insert into internal node: find child to descend into
                while (i>=0 && key < node.keys[i]) i--;
                i++;
                uint64_t childId = node.children[i];
                BTreeNode child = loadNode(childId);
                // If child is full, split it before descending
                if ((int)child.numKeys == MAX_KEYS) {
                    splitChild(node, i, childId);
                    node = loadNode(node.blockId);
                    if (key > node.keys[i]) {
                        i++;
                    }
                }
                // Descend into the appropriate child
                uint64_t newChildId = node.children[i];
                BTreeNode newChild = loadNode(newChildId);
                node = newChild; // Continue insertion in the chosen child
            }
        }
    }

    // Split a full child node into two and adjust the parent node accordingly
    void splitChild(BTreeNode &parent, int index, uint64_t childId) {
        BTreeNode child = loadNode(childId);
        BTreeNode newChild = allocateNode(child.isLeaf);
        newChild.parentId = parent.blockId;

        // Move the upper half of child's keys/values to newChild
        newChild.numKeys = MIN_DEGREE - 1;
        for (int j=0; j<MIN_DEGREE-1; j++) {
            newChild.keys[j] = child.keys[j+MIN_DEGREE];
            newChild.values[j] = child.values[j+MIN_DEGREE];
        }

        // Move the upper half of child's children if not a leaf
        if (!child.isLeaf) {
            for (int j=0; j<MIN_DEGREE; j++) {
                newChild.children[j] = child.children[j+MIN_DEGREE];
                if (newChild.children[j] != 0) {
                    BTreeNode temp = loadNode(newChild.children[j]);
                    temp.parentId = newChild.blockId;
                    saveNode(temp);
                }
            }
        }

        // Adjust the old child node's number of keys
        child.numKeys = MIN_DEGREE - 1;
        saveNode(child);
        saveNode(newChild);

        // Insert newChild into parent
        for (int j=(int)parent.numKeys; j>=index+1; j--) {
            parent.children[j+1] = parent.children[j];
        }
        parent.children[index+1] = newChild.blockId;

        // Move the middle key/value from child to parent
        for (int j=(int)parent.numKeys-1; j>=index; j--) {
            parent.keys[j+1] = parent.keys[j];
            parent.values[j+1] = parent.values[j];
        }
        parent.keys[index] = child.keys[MIN_DEGREE-1];
        parent.values[index] = child.values[MIN_DEGREE-1];

        parent.numKeys++;
        saveNode(parent);
    }

    // Print all keys/values in ascending order (in-order traversal)
    void printInOrder(uint64_t blockId) {
        if (blockId == 0) return; // No node
        BTreeNode node = loadNode(blockId);
        // Traverse the children and keys in order
        for (int i=0; i<(int)node.numKeys; i++) {
            printInOrder(node.children[i]);
            cout << node.keys[i] << " " << node.values[i] << "\n";
        }
        // Print from the last child
        printInOrder(node.children[node.numKeys]);
    }

    // Extract all keys/values in ascending order to a file
    void extractInOrder(uint64_t blockId, ofstream &out) {
        if (blockId == 0) return;
        BTreeNode node = loadNode(blockId);
        for (int i=0; i<(int)node.numKeys; i++) {
            extractInOrder(node.children[i], out);
            out << node.keys[i] << "," << node.values[i] << "\n";
        }
        extractInOrder(node.children[node.numKeys], out);
    }

    // Load key/value pairs from a CSV file and insert them
    void loadFromFile(const string &inputFile) {
        ifstream in(inputFile);
        if (!in.is_open()) {
            throw runtime_error("Unable to open input file for load.");
        }

        string line;
        while (getline(in, line)) {
            if (line.empty()) continue;
            size_t pos = line.find(',');
            if (pos == string::npos) {
                cerr << "Invalid line format in load file: " << line << "\n";
                continue;
            }
            string k = line.substr(0, pos);
            string v = line.substr(pos+1);

            uint64_t key, value;
            try {
                key = stoull(k);
                value = stoull(v);
            } catch (...) {
                cerr << "Invalid integer in line: " << line << "\n";
                continue;
            }

            // Skip if key already exists
            if (keyExists(key)) {
                cerr << "Error: key " << key << " already exists. Skipping.\n";
                continue;
            }
            try {
                insertKey(key, value);
            } catch (runtime_error &e) {
                cerr << "Error inserting key " << key << ": " << e.what() << "\n";
            }
        }
    }

public:
    BTree() {
        fileOpen = false;
        rootBlockId = 0;
        nextBlockId = 1;
    }

    // Create a new B-Tree index file
    void createFile() {
        cout << "Enter the file name to create: ";
        string fname; cin >> fname;
        {
            // Check if file already exists
            ifstream test(fname, ios::binary);
            if (test.is_open()) {
                cout << "File already exists. Overwrite? (y/n): ";
                char c; cin >> c;
                if (c!='y' && c!='Y') {
                    cout << "File creation aborted.\n";
                    return;
                }
            }
        }

        fileName = fname;
        // Create/truncate the file
        file.open(fileName, ios::binary | ios::trunc | ios::in | ios::out);
        if (!file.is_open()) {
            cerr << "Error: Unable to create file.\n";
            return;
        }

        // Initialize empty tree header
        rootBlockId = 0;
        nextBlockId = 1;
        writeHeader();
        fileOpen = true;
        cout << "File created successfully.\n";
    }

    // Open an existing B-Tree index file
    void openFile() {
        cout << "Enter the file name to open: ";
        string fname; cin >> fname;
        fileName = fname;
        file.open(fileName, ios::binary | ios::in | ios::out);
        if (!file.is_open()) {
            cerr << "Error: File does not exist.\n";
            return;
        }
        try {
            readHeader();
        } catch (runtime_error &e) {
            cerr << "Error: " << e.what() << "\n";
            file.close();
            return;
        }

        fileOpen = true;
        cout << "File opened successfully.\n";
    }

    // Insert command: prompt user for key/value and insert
    void insertCommand() {
        if (!fileOpen) {
            cerr << "Error: No index file is open.\n";
            return;
        }
        cout << "Enter key and value: ";
        uint64_t key, value;
        if (!(cin >> key >> value)) {
            cerr << "Error: Invalid input.\n";
            cin.clear(); cin.ignore(10000,'\n');
            return;
        }

        // Check for duplicate
        if (keyExists(key)) {
            cerr << "Error: Key already exists.\n";
            return;
        }

        try {
            insertKey(key, value);
        } catch (runtime_error &e) {
            cerr << "Error: " << e.what() << "\n";
        }
    }

    // Search command: prompt user for key and search the B-Tree
    void searchCommand() {
        if (!fileOpen) {
            cerr << "Error: No index file is open.\n";
            return;
        }
        cout << "Enter key: ";
        uint64_t key;
        if (!(cin >> key)) {
            cerr << "Error: Invalid input.\n";
            cin.clear(); cin.ignore(10000,'\n');
            return;
        }

        if (rootBlockId == 0) {
            cerr << "Error: Key not found.\n";
            return;
        }
        uint64_t value;
        if (searchKey(rootBlockId, key, value)) {
            cout << key << " " << value << "\n";
        } else {
            cerr << "Error: Key not found.\n";
        }
    }

    // Load command: load key/value pairs from a given CSV file
    void loadCommand() {
        if (!fileOpen) {
            cerr << "Error: No index file is open.\n";
            return;
        }
        cout << "Enter the file name to load from: ";
        string fname; cin >> fname;
        try {
            loadFromFile(fname);
        } catch (runtime_error &e) {
            cerr << "Error: " << e.what() << "\n";
        }
    }

    // Print command: print all keys/values in ascending order
    void printCommand() {
        if (!fileOpen) {
            cerr << "Error: No index file is open.\n";
            return;
        }
        if (rootBlockId == 0) {
            // Empty tree, nothing to print
            return;
        }
        printInOrder(rootBlockId);
    }

    // Extract command: write all keys/values to a specified file in ascending order
    void extractCommand() {
        if (!fileOpen) {
            cerr << "Error: No index file is open.\n";
            return;
        }
        cout << "Enter the file name to extract to: ";
        string fname; cin >> fname;

        // Check if file exists
        {
            ifstream test(fname);
            if (test.is_open()) {
                cout << "File already exists. Overwrite? (y/n): ";
                char c; cin >> c;
                if (c!='y' && c!='Y') {
                    cout << "Extract aborted.\n";
                    return;
                }
            }
        }

        ofstream out(fname);
        if (!out.is_open()) {
            cerr << "Error: Unable to open output file.\n";
            return;
        }

        if (rootBlockId != 0) {
            extractInOrder(rootBlockId, out);
        }
        out.close();
        cout << "Extract completed.\n";
    }

    // Close the currently open file and reset state
    void closeFile() {
        if (file.is_open()) {
            file.close();
        }
        fileOpen = false;
        rootBlockId = 0;
        nextBlockId = 1;
    }

    // Destructor: ensure file is closed
    ~BTree() {
        closeFile();
    }
};
