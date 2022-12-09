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
private:
    bool open_;
    int deviceFd;
    unsigned int nsid;
};
