#ifndef HASHMAP_H
#define HASHMAP_H
#include "init_ftl.h"

#define IO_DB_PRINTMAP 0x80
#define IO_DB_INSERT_TABLE 0x84 		// TODO Add to host interface
#define IO_DB_RESET_TABLE_ITERS 0x88 	// TODO Add to host interface
#define IO_DB_PRINT_TABLES 0x8C 		// TODO Add to host interface
#define IO_DB_INSERT 0x81
#define IO_DB_JOIN 0x82 				// TODO Add to host interface
#define IO_DEBUG_CUSTOM_READ 0x86

#define INVALID_HASHMAP_LPN 0xffffffff

typedef union hashKey_ {
	struct {
		unsigned int key;
		unsigned int table;
	} vals;
	unsigned long long hashable;
} hashKey;

typedef struct hashEntry_ {
	unsigned int table : 10;
	unsigned int key : 22;
	unsigned int slb;
} hashEntry;

typedef struct hashMap_ {
	unsigned int bucketLength;	// number of elements in each bucket (hash entries)
	unsigned int bucketCount;
	unsigned int size;			// how many elements are in the hashMap
	unsigned int backingSize;	// size of the backing array
	hashEntry* data;			// backing array
} hashMap;

hashMap makeHashMap(unsigned int bucketLength_, unsigned int bucketCount_, unsigned int backingSize_, void* data_);
void insertHashMap(hashMap* map, unsigned int table, unsigned int key, unsigned int slb);
unsigned int getHashMap(hashMap* map, unsigned int table, unsigned int key);

void InitIndexMap();

#define MAX_BACKING_SIZE PAGE_SIZE * 1024
#define DEFAULT_BUCKET_LENGTH 8
#define DEFAULT_BUCKET_COUNT (MAX_BACKING_SIZE / (DEFAULT_BUCKET_LENGTH * sizeof(struct hashEntry_)))

extern hashMap indexMap;
extern int table1iter;

#endif
