#pragma once
#include <cstddef>
#include <map>
#include <vector>
#include <cstdint>
#include <iostream>

#include "json.hpp"

#include "NVMEDevice.h"

class Table {
public:
    static constexpr std::uint64_t BLOCK_SIZE = 16384;

    enum class ColType : std::uint64_t {
        integer,
        char64,
    };

    struct Column {
        std::string name;
        ColType type;
    };

    Table();
    int getId() const;
    size_t getWidth() const;

    void loadColumns(const nlohmann::json& schemaJson);
    int getColumnCount() const;
    const std::vector<Table::Column>& getColumns() const;

    void insert(const std::vector<std::string>& rowVals, NVMEDevice& device);
    template<typename T>
    void putElement(std::uint64_t row, std::uint64_t col, const T& data);

    void readPage(NVMEDevice& device, int slb);

    std::string toString() const;
    std::string rowToString(std::uint64_t row) const;

private:

    template<typename T>
    void putElement_(std::uint64_t row, std::uint64_t col, const T& data);

    template<typename T>
    void writeElem(std::size_t offset, const T& elem);

    static int currId;
    static unsigned long long currBlock; // the universal block counter

    unsigned long long backingBlock; // the block we're storing
    int id;
    std::vector<Column> columnVec;
    std::vector<char> data;
    std::size_t rowWidth;
    std::size_t maxRows;
    int currRow;
};

template<typename T>
void Table::putElement(std::uint64_t row, std::uint64_t col, const T& elem) {
}

template<typename T>
void Table::putElement_(std::uint64_t row, std::uint64_t col, const T& elem) {
    // TODO add actual exception here
    if(col >= columnVec.size()) {
        std::cout << "Column number " << col << " is invalid.\n";
        throw std::exception{};
    }

    
    std::size_t rowOffset = std::accumulate(columnVec.begin(), columnVec.begin() + col, 0,
            [](std::size_t currVal, Column currCol) {
                switch(currCol.type) {
                case Table::ColType::integer:
                    return currVal + sizeof(std::int64_t);
                case Table::ColType::char64:
                    return currVal + 64;
                }
            });

    // std::cout << "rowWidth: " << rowWidth << ", rowOffset: " << rowOffset << '\n';

    std::size_t offset = rowWidth * row + rowOffset;

    if(offset + sizeof(elem) > data.size()) {
        std::cout << "Attempt to write past end of table.\n";
        //TODO add custom exception
        throw std::exception{};
    }

    writeElem(offset, elem);
}

template<typename T>
void Table::writeElem(std::size_t offset, const T& elem) {
    std::memcpy(data.data() + offset, &elem, sizeof(elem));
}

