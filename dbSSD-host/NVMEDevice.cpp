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

int NVMEDevice::read(char* data, std::uint32_t length, std::uint64_t slb, std::uint16_t nlb) {
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

int NVMEDevice::insertRow(const char* data, std::uint32_t length, std::uint64_t slb,
        std::uint32_t tableId, std::uint32_t key) {
    struct nvme_passthru_cmd cmd = {
        0x81,   // custom insert row command
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
        3,      // user cdwd12
        tableId,      // user cdwd13
        key,      // user cdwd14
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

static int sendDebugCommand(int fd, std::uint8_t command, std::uint32_t nsid,
        std::uint32_t cdwd10 = 0, std::uint32_t cdwd11 = 0, std::uint32_t cdwd12 = 0,
        std::uint32_t cdwd13 = 0, std::uint32_t cdwd14 = 0, std::uint32_t cdwd15 = 0) {

    struct nvme_passthru_cmd cmd = {
        command,   // custom insert row command
        0,      // flags (unused)
        0,      // reserved (unused)
        nsid,      // namespace (default 1)

        0,      // user cdwd2
        0,      // user cdwd3

        0,      // metadata buffer
        0,      // data buffer
        0,      // metadata length
        0,      // data length

        cdwd10,      // user cdwd10
        cdwd11,      // user cdwd11
        cdwd12,      // user cdwd12
        cdwd13,      // user cdwd13
        cdwd14,      // user cdwd14
        cdwd15,      // user cdwd15

        0,      // timeout, in ms, if non-zero does a timeout
        0       // result, set on completion
    };

    int err = ioctl(fd, NVME_IOCTL_IO_CMD, &cmd);
    if(err < 0) {
        perror("NVME IO error");
        throw std::exception{}; //TODO actual exception
    }
    return err;
}

int NVMEDevice::insertTable(std::uint32_t tableId, std::uint32_t tableWidth) {
    return sendDebugCommand(deviceFd, 0x84, nsid, tableId, tableWidth);
}

int NVMEDevice::resetTableIter() {
    return sendDebugCommand(deviceFd, 0x88, nsid);
}

int NVMEDevice::printDeviceTables() {
    return sendDebugCommand(deviceFd, 0x8C, nsid);
}

int NVMEDevice::printTables() {
    return sendDebugCommand(deviceFd, 0x80, nsid);
}


int NVMEDevice::join(char* data, std::uint32_t length, std::uint32_t table1, std::uint32_t table2) {
    struct nvme_passthru_cmd cmd = {
        0x82,   // op code
        0,      // flags (unused)
        0,      // reserved (unused)
        nsid,      // namespace (default 1)

        0,      // user cdwd2
        0,      // user cdwd3

        0,      // metadata buffer
        reinterpret_cast<std::uintptr_t>(data),      // data buffer
        0,      // metadata length
        length,      // data length

        table1,      // user cdwd10
        table2,      // user cdwd11
        0,      // user cdwd12
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
