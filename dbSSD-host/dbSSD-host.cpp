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

using CommandFunction = std::function<int(std::vector<std::string>)>;
using CommandMap = std::unordered_map<std::string, CommandFunction>;
using TableMap = std::unordered_map<int, Table>;
using nlohmann::json;

static CommandMap commandFunctions;
static TableMap tables;

std::vector<std::string> split(std::istream& stream, char c = '\n') {
	std::vector<std::string> lines{};
	for (std::string s; std::getline(stream, s, c); lines.push_back(s));
	return lines;
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

    std::cout << "New table ID is " << table.getId() << ".\n";

    return 0;
}

int commandInsertRow(const std::vector<std::string>& command) {
    if(command.size() < 3) {
        std::cout << "Invalid command length\n";
        return 1;
    }

    int tableId = std::stoi(command[1]);
    Table& targetTable = tables.at(tableId);

    if(command.size() != 2 + targetTable.getColumnCount()) {
        std::cout << "Invalid number of insert elements\n";
        return 1;
    }

    std::vector<std::string> rowVals{command.begin() + 2, command.end()};
    targetTable.insert(rowVals);

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
    }
    int tableId = std::stoi(command[1]);
    int row = std::stoi(command[2]);

    std::cout << "Table: " << tableId << ", row: " << row << ", " << tables.at(tableId).rowToString(row) << '\n';
    return 0;
}

int main(int argc, char** argv) {
    commandFunctions.emplace("exit", commandExit);
    commandFunctions.emplace("create-table", commandCreateTable);
    commandFunctions.emplace("print-tables", commandPrintTables);
    commandFunctions.emplace("insert-row", commandInsertRow);
    commandFunctions.emplace("print-row", commandPrintRow);
    std::cout << "Welcome to dbSSD-host.\n";
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
