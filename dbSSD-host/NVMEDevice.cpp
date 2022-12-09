#include <cstddef>
#include <cstdint>
#include <exception>

#include "NVMEDevice.h"

NVMEDevice::NVMEDevice() :
    deviceFd{},
    nsid{},
    open_{false}
{}

void NVMEDevice::load(const std::string& device) {
    deviceFd = open(device.c_str(), O_RDWR);
    if(deviceFd < 0) {
        std::string err = "Error opening device " + device;
        perror(err.c_str());
        throw std::exception{};
    }

    if(nvme_get_nsid(deviceFd, &nsid) < 0) {
        std::string err = "Error retrieving namespace Id";
        perror(err.c_str());
        throw std::exception{};
    }
    open_ = true;
}

int NVMEDevice::write(const char* data, std::uint32_t length, std::uint64_t slb, std::uint16_t nlb) {
    struct nvme_passthru_cmd cmd = {
        0x02,   // op code
        0,      // flags (unused)
        0,      // reserved (unused)
        nsid,      // namespace (default 1)

        0,      // user cdwd2
        0,      // user cdwd3

        0,      // metadata buffer
        reinterpret_cast<std::uintptr_t>(data),      // data buffer
        0,      // metadata length
        length,      // data length

        static_cast<std::uint32_t>(slb),      // user cdwd10
        static_cast<std::uint32_t>(slb << 32),      // user cdwd11
        static_cast<std::uint32_t>(nlb),      // user cdwd12
        0,      // user cdwd13
        0,      // user cdwd14
        0,      // user cdwd15

        0,      // timeout, in ms, if non-zero does a timeout
        0       // result, set on completion
    };

    int err = ioctl(deviceFd, NVME_IOCTL_IO_CMD, &cmd);
    if(err < 0) {
        perror("NVME IO error");
        throw std::exception{}; //TODO actual exception
    }
    return err;
}

int NVMEDevice::read(char* data, std::uint32_t length, std::uint64_t slb, std::uint16_t nlb) {
    struct nvme_passthru_cmd cmd = {
        0x01,   // op code
        0,      // flags (unused)
        0,      // reserved (unused)
        nsid,      // namespace (default 1)

        0,      // user cdwd2
        0,      // user cdwd3

        0,      // metadata buffer
        reinterpret_cast<std::uintptr_t>(data),      // data buffer
        0,      // metadata length
        length,      // data length

        static_cast<std::uint32_t>(slb),      // user cdwd10
        static_cast<std::uint32_t>(slb << 32),      // user cdwd11
        static_cast<std::uint32_t>(nlb),      // user cdwd12
        0,      // user cdwd13
        0,      // user cdwd14
        0,      // user cdwd15

        0,      // timeout, in ms, if non-zero does a timeout
        0       // result, set on completion
    };

    int err = ioctl(deviceFd, NVME_IOCTL_IO_CMD, &cmd);
    if(err < 0) {
        perror("NVME IO error");
        throw std::exception{}; //TODO actual exception
    }
    return err;
}

bool NVMEDevice::isOpen() const {
    return open_;
}
