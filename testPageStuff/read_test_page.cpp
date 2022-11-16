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
    if(argc != 2) {
        std::cout << "Incorrect number of args\n";
    }
    std::cout << "Reading from file " << argv[1] << '\n';
    std::fstream file{{argv[1]}, std::ios::in | std::ios::binary};
    if(file.good()) {
        printPage(file);
    }
    return 0;
}
