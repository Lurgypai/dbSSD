#!/bin/python3

import sys

if __name__ == "__main__":
    elemCount = int(sys.argv[1])
    rowCount = int(sys.argv[2])

    field_list = ""
    for i in range(1, elemCount - 1):
        field_list += '    "field' + str(i) + '": "integer",\n'
    field_list += '    "field' + str(elemCount - 1) + '": "integer"\n'

    template_file = open("testTable-template", "r")
    template_file_contents = template_file.read()
    template_file_contents = template_file_contents.replace("<field-list>", field_list)

    output_table_file_name = "testTable-" + str(elemCount) + "e-" + str(rowCount) + "r.json"
    output_table_file = open(output_table_file_name, "w")
    output_table_file.write(template_file_contents)


    file = open("testScript-" + str(elemCount) + "e-" + str(rowCount) + "r", "w")
    file.write("open-device /dev/nvme0n1\n")
    file.write("create-table " + output_table_file_name + "\n")
    file.write("create-table " + output_table_file_name + "\n")
    
    row_contents = ""
    for i in range(1, elemCount):
        row_contents += " " + str(i)

    for i in range(0, rowCount):
        file.write("insert-row 1 " + str(i) + row_contents + "\n")
        file.write("insert-row 2 " + str(i) + row_contents + "\n")
    file.write("join 1 2 " + str(elemCount) + "e-" + str(rowCount) + "r.out\n")
    file.write("exit\n")
