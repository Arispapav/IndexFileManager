# B-Tree Index File Project

## Files and Their Roles
- **main.cpp**:  
  Contains the `main()` function and the user interaction loop. It handles user commands, calls corresponding B-Tree operations, and manages file opening and closing.

- **Btree.cpp**:  
  Implements the `BTree` class and all its related operations:
  - Creating and opening index files.
  - Inserting keys and values.
  - Searching for keys.
  - Loading keys/values from a CSV file.
  - Printing keys/values in ascending order.
  - Extracting keys/values to a file.
  
  It includes logic for reading/writing nodes to disk, maintaining the header block, and ensuring keys are stored in big-endian format.

- **writeIndex.cpp**:  
  Provides utility functions for converting between host-endian and big-endian formats. These ensure correct byte ordering when reading and writing integers to the index file.

## Compilation Instructions
Make sure all three files (`main.cpp`, `Btree.cpp`, and `writeIndex.cpp`) are in the same directory.

Compile using:
```bash
g++ main.cpp -o btree_program
