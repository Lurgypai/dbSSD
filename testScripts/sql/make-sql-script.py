#!/bin/python3

import sys

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Incorrect arg count!!!!!!")

    elemCount = int(sys.argv[1])
    rowCount = int(sys.argv[2])

    table_layout = "\tid bigint PRIMARY KEY,\n"
    for i in range(1, elemCount-1):
        table_layout += "\tfield" + str(i) + " bigint,\n"
    table_layout += "\tfield" + str(elemCount - 1) + " bigint"

    insert_table3 = "INSERT INTO table3 VALUES(n, "
    for i in range(1, elemCount - 1):
        insert_table3 += str(i) + ", "
    insert_table3 += str(elemCount - 1) + ");"

    insert_table4 = "INSERT INTO table4 VALUES(n, "
    for i in range(1, elemCount - 1):
        insert_table4 += str(i) + ", "
    insert_table4 += str(elemCount - 1) + ");"

    dummy = open("testScript-dummy", "r")
    dummy_contents = dummy.read()

    dummy_contents = dummy_contents.replace("<table-layout>", table_layout);
    dummy_contents = dummy_contents.replace("<insert-table3>", insert_table3);
    dummy_contents = dummy_contents.replace("<insert-table4>", insert_table4);
    dummy_contents = dummy_contents.replace("<row-count>", str(rowCount));

    out = open("testSql-" + str(elemCount) + "e-" + str(rowCount) + "r.sql", "w")
    out.write(dummy_contents)

