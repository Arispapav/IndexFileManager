#include <iostream>
#include <fstream>
#include <cstring>

using namespace std;

const string MAGIC_NUMBER = "4337PRJ3"; // 8 bytes
const int HEADER_SIZE = 512;

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

int main() {
    // Main Driver Program
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
    }

    return 0;
}