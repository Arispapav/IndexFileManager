#include <iostream>
#include <fstream>
#include <cstring>
#include <iomanip>
#include "writeIndex.cpp"
#include "Btree.cpp"
using namespace std;

const string MAGIC_NUMBER = "4337PRJ3"; // 8 bytes
const int HEADER_SIZE = 512;
void generateHexDump(const string, const string );

// Function to create a new index file
void createIndexFile() {
    string fileName;
    cout << "Enter the file name to create: ";
    cin >> fileName;

    // Check if file already exists
    ifstream existingFile(fileName, ios::binary);
    if (existingFile.is_open()) {
        cout << "File already exists. Overwrite? (y/n): ";
        char choice;
        cin >> choice;
        if (choice != 'y' && choice != 'Y') {
          cout << "File creation aborted.\n";
            return;
        }
        existingFile.close();
    }

    // Create the file
    ofstream file(fileName, ios::binary | ios::trunc);
    if (!file.is_open()) {
        cerr << "Error: Unable to create file.\n";
        return;
    }

    // Write header block
    char header[HEADER_SIZE] = {0};
    memcpy(header, MAGIC_NUMBER.c_str(), MAGIC_NUMBER.size()); // Magic number
    uint64_t rootNodeId = 0;
    uint64_t nextBlockId = 1;
    memcpy(header + 8, &rootNodeId, sizeof(rootNodeId));        // Root node ID
    memcpy(header + 16, &nextBlockId, sizeof(nextBlockId));    // Next block ID

    file.write(header, HEADER_SIZE);
    file.close();

    cout << "File created successfully.\n";
}

// Function to open an existing index file
void openIndexFile() {
    string fileName;
    cout << "Enter the file name to open: ";
    cin >> fileName;

    ifstream file(fileName, ios::binary);
    if (!file.is_open()) {
        cerr << "Error: File does not exist.\n";
        return;
    }

    // Read and validate the header
    char header[HEADER_SIZE] = {0};
    file.read(header, HEADER_SIZE);
    if (file.gcount() < HEADER_SIZE) {
        cerr << "Error: Invalid file format (header too short).\n";
        file.close();
        return;
    }

    // Check magic number
    if (strncmp(header, MAGIC_NUMBER.c_str(), MAGIC_NUMBER.size()) != 0) {
        cerr << "Error: Magic number mismatch. Not a valid index file.\n";
        file.close();
        return;
    }

    cout << "File opened successfully.\n";
    file.close();
}

// Convert a uint64_t value to big-endian byte array
void toBigEndian(uint64_t value, uint8_t *buffer) {
    for (int i = 7; i >= 0; --i) {
        buffer[i] = value & 0xFF;
        value >>= 8;
    }
}

// Convert a big-endian byte array to uint64_t
uint64_t fromBigEndian(const uint8_t *buffer) {
    uint64_t value = 0;
    for (int i = 0; i < 8; ++i) {
        value = (value << 8) | buffer[i];
    }
    return value;
}

int main() {
    /* Main Driver Program
    while (true) {
        cout << "Commands:\n";
        cout << "1. create - Create a new index file\n";
        cout << "2. open - Open an existing index file\n";
        cout << "3. quit - Exit the program\n";
        cout << "Enter a command: ";

        string command;
        cin >> command;

        if (command == "create") {
            createIndexFile();
        }
        else if (command == "open") {
            openIndexFile();
        } 
        else if (command == "quit") {
            cout << "Exiting the program.\n";
            break;
        } 
        else {
            cout << "Invalid command. Please try again.\n";
        }
    } */ 
    // Testing B-Tree 
      BTree btree("btree.dat");
    string outputFile = "btree_hex_dump.txt";
    btree.insert(10, 100);
    btree.insert(20, 200);
    btree.insert(30, 300);

    if (btree.search(20)) {
        std::cout << "Search successful.\n";
    } else {
        std::cout << "Key not found.\n";
    }
    generateHexDump("btree.dat", outputFile);

    return 0;
    
}



void generateHexDump(const std::string &binaryFile, const std::string &outputFile) {
    std::ifstream input(binaryFile, std::ios::binary);
    if (!input.is_open()) {
        std::cerr << "Error: Unable to open binary file: " << binaryFile << "\n";
        return;
    }

    std::ofstream output(outputFile);
    if (!output.is_open()) {
        std::cerr << "Error: Unable to open output file: " << outputFile << "\n";
        return;
    }

    uint8_t buffer[16]; // Read 16 bytes at a time
    size_t offset = 0;

    output << "Hex Dump of " << binaryFile << ":\n";
    while (input.read(reinterpret_cast<char *>(buffer), sizeof(buffer)) || input.gcount() > 0) {
        // Print offset
        output << std::setw(8) << std::setfill('0') << std::hex << offset << ": ";

        // Print bytes in hex
        for (size_t i = 0; i < sizeof(buffer); ++i) {
            if (i < static_cast<size_t>(input.gcount())) {
                output << std::setw(2) << static_cast<int>(buffer[i]) << " ";
            } else {
                output << "   "; // Empty space for alignment
            }
        }

        // Print ASCII representation
        output << " |";
        for (size_t i = 0; i < static_cast<size_t>(input.gcount()); ++i) {
            if (std::isprint(buffer[i])) {
                output << static_cast<char>(buffer[i]);
            } else {
                output << ".";
            }
        }
        output << "|\n";

        offset += sizeof(buffer);
    }

    input.close();
    output.close();

    std::cout << "Hex dump generated in file: " << outputFile << "\n";
}

