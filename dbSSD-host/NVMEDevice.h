#pragma once

#include <string>
#include <fcntl.h>
#include <libnvme.h>

class NVMEDevice {
public:
    NVMEDevice();

    void load(const std::string& device);
    int write(const char* data, std::uint32_t length, std::uint64_t slb, std::uint16_t nlb);
    int read(char* data, std::uint32_t length, std::uint64_t slb, std::uint16_t nlb);

    bool isOpen() const;

    // custom commands
    int printTables();

    int insertRow(const char* data, std::uint32_t length, std::uint64_t slb,
            std::uint32_t tableId, std::uint32_t key);
    int insertTable(std::uint32_t tableId, std::uint32_t tableWidth);
    int resetTableIter();
    int printDeviceTables();

    int join(char* data, std::uint32_t length, std::uint32_t table1, std::uint32_t table2);

private:
    bool open_;
    int deviceFd;
    unsigned int nsid;
};
