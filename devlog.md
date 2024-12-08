Log Entry: 12/8/2024 5:05 PM
Thoughts on project
Project from my understanding of reading the documentation, is to create a program that manages index files using a 
B-tree stucture. Key functionalities I will have to consider are creating, opening, inserting, searching, and extracting data.
I will also need to make to use only 3 nodes max in memory at a time. 
5 main points of percision are
1. File I/O mangement
2. B-tree implementation
3. User Interface
4. Data encoding
5. Persitance and Consistency

Inital Plan
1. Implement the file creation and opening commands to handle the header structure and basic file validation
2. write functions for converting to and from big-endian and mapping 512-byte blocks
3. Design the B-tree and implement needed methods like insert and search functionaltiies
4. Make sure in-memory operations work then try disk-
5. Develop each command iteratively: create, open, insert, search, load, print, extract, and quit
6. After each step make sure to perform tests to ensure functionality, also going to be using online gdb for compliation and
   debugging so ensure it works in cs1 and cs2 will be final step.
