#include "table.h"
#include "memory_map.h"
#include "debug.h"

table_p tables = TABLE_ARR_ADDR;

void insertTable(unsigned int id, unsigned int rowWidth) {
	if(id >= TABLE_COUNT) {
		xil_printf("Table id %d is too large.\r\n", id);
		ASSERT(0);
	}

	table t = {
			.id = id,
			.rowWidth = rowWidth
	};

	tables[id] = t;
}

table_p getTable(unsigned int id) {
	if(id >= TABLE_COUNT) {
		xil_printf("Could not get table %d, id too large.\r\n", id);
		ASSERT(0);
	}
	return &tables[id];
}
