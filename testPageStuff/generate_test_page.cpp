#include <iostream>
#include <cwchar>
#include <fstream>
#include <cstdint>
#include <vector>
#include <string>
#include <cstring>

// data count and offset need to be stored on the page

std::uint64_t dataCount; // number of data items written
std::uint32_t dataOffset; // offset from the end to write new data entries

enum DataType : uint32_t {
    NONE = 0,
    UINT64 = 1,
};

struct DataEntry {
    DataType type;
    std::uint32_t offset;
    std::uint32_t length;
};

template<typename T>
DataType getDataType() {
    return DataType::NONE;
}

template<>
DataType getDataType<std::uint64_t>() {
    return DataType::UINT64;
}

template<typename T>
void writeData(std::fstream& file, const T& data) {
    dataOffset -= sizeof(T);

    DataEntry d{getDataType<T>(), dataOffset, sizeof(T)};

    file.seekp(sizeof(dataCount) + sizeof(dataOffset) + dataCount * sizeof(DataEntry));
    file.write(reinterpret_cast<char*>(&d), sizeof(DataEntry));

    file.seekp(dataOffset, std::ios::end);
    file.write(reinterpret_cast<const char*>(&data), sizeof(T)); 

    ++dataCount;

    file.seekp(0);
    file.write(reinterpret_cast<char*>(&dataCount), sizeof(dataCount));
    file.write(reinterpret_cast<char*>(&dataOffset), sizeof(dataOffset));
}

void printPage(std::fstream& file) {
    std::vector<DataEntry> dataEntries{};

    file.seekg(0);
    decltype(dataCount) dataCount_;
    decltype(dataOffset) dataOffset_;
    file.read(reinterpret_cast<char*>(&dataCount_), sizeof(dataCount_)); 
    file.read(reinterpret_cast<char*>(&dataOffset_), sizeof(dataOffset_)); 

    std::cout << "dataCount:" << dataCount_ << '\n';
    std::cout << "dataOffset:" << dataOffset_ << '\n';
    
    for(auto i = 0; i != dataCount_; ++i) {
        DataEntry dataEntry{};
        file.read(reinterpret_cast<char*>(&dataEntry), sizeof(DataEntry)); 
        dataEntries.emplace_back(std::move(dataEntry));
    }

    for(auto& dataEntry : dataEntries) {
        std::cout << "DataEntry:\n";
        std::cout << "\ttype: " << dataEntry.type << '\n';
        std::cout << "\toffset: " << dataEntry.offset << '\n';
        std::cout << "\tlength: " << dataEntry.length << '\n';
    }
}

int main(int argc, char** argv) {
    std::fstream file{"test_file", std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc};
    if(file.good()) {
        // allocate 4 kb of space
        char zeroes[1024 * 4] = {0}; 
        file.write(zeroes, 4 * 1024);
        dataOffset = 4 * 1024;
        dataCount = 0;

        // write out some test data
        writeData<std::uint64_t>(file, 0);
        writeData<std::uint64_t>(file, 1);
        writeData<std::uint64_t>(file, 2);
        printPage(file);
    }
    return 0;
}
