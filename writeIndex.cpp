#include <iostream>
#include <fstream>
#include <cstdint>
#include <cstring>
#include <vector>


// Utility class to manage 512-byte blocks
class BlockManager {
public:
    static const size_t BLOCK_SIZE = 512;

    BlockManager(const std::string &fileName) : fileName(fileName) {}

    // Write a block of data at the specified block ID
    void writeBlock(uint64_t blockId, const uint8_t *data) {
        std::ofstream file(fileName, std::ios::binary | std::ios::in | std::ios::out);
        if (!file.is_open()) {
            throw std::runtime_error("Unable to open file for writing.");
        }
        file.seekp(blockId * BLOCK_SIZE);
        file.write(reinterpret_cast<const char *>(data), BLOCK_SIZE);
        file.close();
    }

    // Read a block of data from the specified block ID
    void readBlock(uint64_t blockId, uint8_t *data) {
        std::ifstream file(fileName, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Unable to open file for reading.");
        }
        file.seekg(blockId * BLOCK_SIZE);
        file.read(reinterpret_cast<char *>(data), BLOCK_SIZE);
        file.close();
    }

private:
    std::string fileName;
};


