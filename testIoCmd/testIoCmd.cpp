#include <libnvme.h>
#include <fcntl.h>
#include <errno.h>
#include <iostream>
#include <cstdint>

int main(int argc, char** argv) {
    if(argc == 1) {
        std::cout << "Missing file.\n";
        return -1;
    }

    int fd = open(argv[1], O_RDWR);
    if(fd < 0) perror("File error");

    unsigned int nsid;
    if(nvme_get_nsid(fd, &nsid) < 0) perror("nsid error");
    std::cout << "Namespace is: " << nsid << '\n';

    struct nvme_passthru_cmd cmd = {
        0x80,   // op code
        0,      // flags (unused)
        0,      // reserved (unused)
        nsid,      // namespace (default 1)

        0,      // user cdwd2
        0,      // user cdwd3

        0,      // metadata buffer
        0,      // data buffer
        0,      // metadata length
        0,      // data length

        0,      // user cdwd10
        0,      // user cdwd11
        0,      // user cdwd12
        0,      // user cdwd13
        0,      // user cdwd14
        0,      // user cdwd15

        0,      // timeout, in ms, if non-zero does a timeout
        0       // result, set on completion
    };

    int err = ioctl(fd, NVME_IOCTL_IO_CMD, &cmd);
    if(err < 0) perror("NVME IO error");
}
