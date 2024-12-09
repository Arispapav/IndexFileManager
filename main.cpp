#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <string>
#include <algorithm>
#include <stdexcept>
#include <vector>
#include <cctype>

// Include the helper and B-tree code
#include "writeIndex.cpp"
#include "Btree.cpp"

using namespace std;

int main() {
    BTree btree;
    while (true) {
        cout << "\nCommands:\n";
        cout << "  create\n";
        cout << "  open\n";
        cout << "  insert\n";
        cout << "  search\n";
        cout << "  load\n";
        cout << "  print\n";
        cout << "  extract\n";
        cout << "  quit\n";
        cout << "Enter a command: ";

        string command;
        cin >> command;

        // Convert command to lowercase
        for (auto &ch : command) ch = (char)tolower((unsigned char)ch);

        if (command == "create") {
            btree.createFile();
        } 
        else if (command == "open") {
            btree.closeFile();
            btree.openFile();
        }
        else if (command == "insert") {
            btree.insertCommand();
        }
        else if (command == "search") {
            btree.searchCommand();
        }
        else if (command == "load") {
            btree.loadCommand();
        } 
        else if (command == "print") {
            btree.printCommand();
        } 
        else if (command == "extract") {
            btree.extractCommand();
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
