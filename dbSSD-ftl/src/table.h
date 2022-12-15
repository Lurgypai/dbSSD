#ifndef TABLE_H
#define TABLE_H

typedef struct table_ {
	unsigned int id;
	unsigned int rowWidth;
} table;
typedef table* table_p;

extern table_p tables;

void insertTable(unsigned int id, unsigned int rowWidth);
table_p getTable(unsigned int id);

#define TABLE_COUNT 16

#endif
