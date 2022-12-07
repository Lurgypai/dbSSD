#include "table.h"
#include <algorithm>
#include <cstring>
#include <numeric>
#include <sstream>
#include <iostream>
#include <exception>

using nlohmann::json;

Table::Table() :
    id{++currId},
    currRow{0},
    data{}
{
    data.resize(BLOCK_SIZE);
}
int Table::getId() const {
    return id;
}

void Table::loadColumns(const json& schemaJson) {
    for(auto it = schemaJson.begin(); it != schemaJson.end(); ++it) {
        std::string val{it.value()};
        if(val == "integer") {
            columnVec.emplace_back(std::string{it.key()}, Table::ColType::integer);
        }
        else if (val == "char64") {
            columnVec.emplace_back(std::string{it.key()}, Table::ColType::char64);
        }
        else {
            std::cout << "Unable to parse column type: " << val << ".\n";
            throw std::exception{}; // TODO: Make this a custom exception type
        }
    }
}

int Table::getColumnCount() const {
    return columnVec.size();
}

template<>
void Table::putElement<std::int64_t>(std::uint64_t row, std::uint64_t col, const std::int64_t& elem) {
    Table::ColType targetColType = columnVec[col].type;
    if(targetColType != Table::ColType::integer) { 
        std::cout << "Column " << col << " is not an integer column.\n";
        throw std::exception{};
    }

    putElement_(row, col, elem);
}

template<>
void Table::putElement<std::string>(std::uint64_t row, std::uint64_t col, const std::string& elem) {
    Table::ColType targetColType = columnVec[col].type;
    if(targetColType != Table::ColType::char64) { 
        std::cout << "Column " << col << " is not a string column.\n";
        throw std::exception{};
    }

    putElement_(row, col, elem);
}

void Table::insert(const std::vector<std::string>& colVals) {
    int col = 0;
    for(const auto& val : columnVec) {
        switch(val.type) {
            case Table::ColType::integer: {
                std::int64_t elem = std::stoi(colVals[col]);
                putElement(Table::currRow, col, elem);
                } break;

            case Table::ColType::char64: {
               putElement(currRow, col, colVals[col]);
               } break;
        } 
        ++col;
    }

    ++currRow;
}

std::string Table::toString() const {
    std::stringstream ss;
    ss << "Table ID: " << id;
    for(const auto& [name, type] : columnVec) {
        ss << ", [Column: " << name << ", Type: " << static_cast<std::uint64_t>(type) << ']';
    }
    return ss.str();
}

std::string Table::rowToString(std::uint64_t row) const {
    std::stringstream ss;
    std::size_t offset = 0;

    std::size_t rowWidth = std::accumulate(columnVec.begin(), columnVec.end(), 0, [](std::size_t currVal, Column currCol) {
                switch(currCol.type) {
                case Table::ColType::integer:
                    return currVal + sizeof(std::int64_t);
                case Table::ColType::char64:
                    return currVal + 64;
                }
            });

    for(const auto& column : columnVec) {
        switch (column.type) {
            case Table::ColType::integer: {
                std::int64_t elem;
                std::memcpy(&elem, data.data() + offset + (rowWidth * row), sizeof(elem));
                ss << elem << ", ";
                offset += sizeof(elem);
                } break;
            case Table::ColType::char64: {
                std::string elem;
                elem.resize(64);
                std::memcpy(&elem[0], data.data() + offset + (rowWidth * row), 64);
                ss << elem << ", ";
                offset += 64;
                } break;
        }
    }
    return ss.str();
}

template<>
void Table::writeElem<std::string>(std::size_t offset, const std::string & elem) {
    std::memcpy(data.data() + offset, elem.data(), elem.size());
}

int Table::currId{};
unsigned long long Table::currBlock{};
