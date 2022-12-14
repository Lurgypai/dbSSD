#!/bin/python3

import sys

if __name__ == "__main__":
    rowCount = int(sys.argv[1])
    file = open("testScript-" + str(rowCount) + "-lines", "w")
    file.write("open-device /dev/nvme0n1\n")
    file.write("create-table testTable.json\n")
    file.write("create-table testTable.json\n")
    for i in range(0, rowCount):
        file.write("insert-row 1 " + str(i) + " 0 1 2 3 4 5 6 7 8\n")
        file.write("insert-row 2 " + str(i) + " 0 1 2 3 4 5 6 7 8\n")
    file.write("join 1 2\n")
    file.write("exit\n")
