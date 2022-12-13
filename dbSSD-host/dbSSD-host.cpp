#include <iostream>
#include <ranges>
#include <string>
#include <sstream>
#include <unordered_map>
#include <functional>
#include <vector>
#include <fstream>

#include "json.hpp"

#include "table.h"
#include "NVMEDevice.h"

using CommandFunction = std::function<int(std::vector<std::string>)>;
using CommandMap = std::unordered_map<std::string, CommandFunction>;
using TableMap = std::unordered_map<int, Table>;
using nlohmann::json;

static CommandMap commandFunctions;
static TableMap tables;
static NVMEDevice device;

std::vector<std::string> split(std::istream& stream, char c = '\n') {
	std::vector<std::string> lines{};
	for (std::string s; std::getline(stream, s, c); lines.push_back(s));
	return lines;
}

int commandHelp(const std::vector<std::string>& command) {
    std::cout << "The following are the available commands.\n";
    for(auto& [command, _] : commandFunctions) {
        std::cout << command << '\n';
    }
    return 0;
}

int commandExit(const std::vector<std::string>& command) {
    std::cout << "Exiting...\n";
    exit(0);
    return 0; // redundant
}

int commandCreateTable(const std::vector<std::string>& command) {
    if(command.size() != 2) {
        std::cout << "Invalid command length\n";
        return 1;
    }

    if(!device.isOpen()) { 
        std::cout << "Haven't opened an NVME device!\n";
        return 1;
    }

    std::cout << "Generating table from schema file " << command[1] << ".\n";
    std::fstream schemaFile{command[1]};
    if(!schemaFile.good()) {
        std::cout << "Unable to open file " << command[1] << '\n';
        return 1;
    }

    json schemaJson{};
    schemaFile >> schemaJson;

    Table table{};
    table.loadColumns(schemaJson);
    tables.emplace(table.getId(), table);

    device.insertTable(table.getId(), table.getWidth());

    std::cout << "New table ID is " << table.getId() << ".\n";

    return 0;
}

int commandInsertRow(const std::vector<std::string>& command) {
    if(command.size() < 3) {
        std::cout << "Invalid command length\n";
        return 1;
    }

    if(!device.isOpen()) { 
        std::cout << "Haven't opened an NVME device!\n";
        return 1;
    }

    int tableId = std::stoi(command[1]);
    Table& targetTable = tables.at(tableId);

    if(command.size() != 2 + targetTable.getColumnCount()) {
        std::cout << "Invalid number of insert elements\n";
        return 1;
    }

    std::vector<std::string> rowVals{command.begin() + 2, command.end()};
    targetTable.insert(rowVals, device);

    std::cout << "Inserted row into table " << tableId << '\n';

    return 0;
}

int commandLoadTablePage(const std::vector<std::string>& command) {
    if(command.size() != 3) {
        std::cout << "Invalid command length\n";
        return 1;
    }

    if(!device.isOpen()) { 
        std::cout << "Haven't opened an NVME device!\n";
        return 1;
    }
    
    int tableId = std::stoi(command[1]);
    int slb = std::stoi(command[2]);

    auto& targetTable = tables.at(tableId);
    targetTable.readPage(device, slb);

    std::cout << "Read page " << slb << " into current page for table " << tableId << '\n';
    return 0;
}

int commandPrintTables(const std::vector<std::string>& command) {
    for(const auto& tablePair : tables) {
        std::cout << tablePair.second.toString() << '\n';
    }
    return 0;
}

int commandPrintRow(const std::vector<std::string>& command) {
    if(command.size() != 3) {
        std::cout << "Invalid command length\n";
        return 1;
    }
    int tableId = std::stoi(command[1]);
    int row = std::stoi(command[2]);

    std::cout << "Table: " << tableId << ", row: " << row << ", " << tables.at(tableId).rowToString(row) << '\n';
    return 0;
}

int commandOpenDevice(const std::vector<std::string>& command) {
    if(command.size() != 2) {
        std::cout << "Invalid command length\n";
        return 1;
    }

    device.load(command[1]);
    std::cout << "Opened NVMe device " << command[1] << '\n';
    return 0;
}

int commandPrintDeviceTables(const std::vector<std::string>& command) {
    device.printDeviceTables();
    std::cout << "Sent print table command to device\n";
    return 0;
}

int commandJoin(const std::vector<std::string>& command) {
    if(command.size() != 3) {
        std::cout << "Invalid command length\n";
        return 1;
    }

    std::uint32_t table1 = std::stoi(command[1]);
    std::uint32_t table2 = std::stoi(command[2]);
    std::cout << "Joining tables " << table1 << " and " << table2 << '\n';

    std::vector<char> data;
    data.resize(16384); // page size

    device.resetTableIter();
    device.join(data.data(), data.size(), table1, table2);

    std::cout << "Join complete:\n";

    Table& targetTable1 = tables.at(table1);
    Table& targetTable2 = tables.at(table2); 

  
    // print a row
    size_t offset = 0; 
    for(auto&& [_, type] : targetTable1.getColumns()) {
        if(type == Table::ColType::integer) {
            std::int64_t val;
            std::memcpy(&val, data.data() + offset, sizeof(val));
            std::cout << val << ", ";
            offset += sizeof(val);
        }
        if(type == Table::ColType::char64) {
            std::string val;
            val.resize(64);
            std::memcpy(val.data(), data.data() + offset, 64);
            std::cout << val << ", ";
            offset += 64;
        }
    }

    // skip key
    auto& table2Columns = targetTable2.getColumns();
    for(int i = 1; i != table2Columns.size(); ++i) {
        const auto& type = table2Columns[i].type;
        if(type == Table::ColType::integer) {
            std::int64_t val;
            std::memcpy(&val, data.data() + offset, sizeof(val));
            std::cout << val << ", ";
            offset += sizeof(val);
        }
        if(type == Table::ColType::char64) {
            std::string val;
            val.resize(64);
            std::memcpy(val.data(), data.data() + offset, 64);
            std::cout << val << ", ";
            offset += 64;
        }
    }
    std::cout << '\n';
    
    return 0;
}

int commandPrintDeviceHashmap(const std::vector<std::string>& command) {
    device.printTables();
    std::cout << "Sent print map command to device\n";
    return 0;
}

int main(int argc, char** argv) {
    std::cout << "Welcome to dbSSD-host.\n";
    commandFunctions.emplace("help", commandHelp);
    commandFunctions.emplace("exit", commandExit);
    commandFunctions.emplace("create-table", commandCreateTable);
    commandFunctions.emplace("print-tables", commandPrintTables);
    commandFunctions.emplace("insert-row", commandInsertRow);
    commandFunctions.emplace("print-row", commandPrintRow);
    commandFunctions.emplace("open-device", commandOpenDevice);
    commandFunctions.emplace("load-table-page", commandLoadTablePage);
    commandFunctions.emplace("print-device-tables", commandPrintDeviceTables);
    commandFunctions.emplace("print-device-hashmap", commandPrintDeviceHashmap);
    commandFunctions.emplace("join", commandJoin);

    if(argc == 2) {
        std::ifstream file{argv[1]};
        if(file.good()) {
            
            std::string line;
            while(std::getline(file, line)) {
                std::stringstream ss{line};
                std::vector<std::string> tokens = split(ss, ' ');
                auto commandFunc = commandFunctions.find(tokens[0]);
                if(commandFunc != commandFunctions.end()) {
                    commandFunc->second(tokens);
                }
                else {
                    std::cout << "Invalid command: " << tokens[0] << '\n';
                }
            }
        } else {
            std::cout << "Unable to open file " << argv[1] << " exiting...\n";
            return 1;
        }
    }

    std::string line;
    while(std::getline(std::cin, line)) {
        std::stringstream ss{line};
        std::vector<std::string> tokens = split(ss, ' ');
        auto commandFunc = commandFunctions.find(tokens[0]);
        if(commandFunc != commandFunctions.end()) {
            commandFunc->second(tokens);
        }
        else {
            std::cout << "Invalid command: " << tokens[0] << '\n';
        }
    }
}
