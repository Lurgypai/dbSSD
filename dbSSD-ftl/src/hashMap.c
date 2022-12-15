#include "hashmap.h"
#include "debug.h"
#include "xil_printf.h"
#include "memory_map.h"

// TODO make this smart
static unsigned long long hash(unsigned int table, unsigned int key) {
	hashKey k;
	k.vals.table = table;
	k.vals.key = key;
	return k.hashable;
}

hashMap makeHashMap(unsigned int bucketLength_, unsigned int bucketCount_, unsigned int backingSize_, void* data_) {
	hashMap map = {bucketLength_, bucketCount_, 0, backingSize_, data_};

	int i = 0;
	for(; i != map.backingSize; ++i) {
		map.data[i].key = 0;
		map.data[i].slb = INVALID_HASHMAP_LPN;
	}
	return map;
}


void insertHashMap(hashMap* map, unsigned int table, unsigned int key, unsigned int slb) {

	unsigned long long hashValue = hash(table, key);
	unsigned int bucketId = hashValue % ((unsigned long long)map->bucketCount);

	unsigned int i = 0;
	for(; i != map->bucketLength; ++i) {
		unsigned int index = bucketId * map->bucketLength + i;
		if(map->data[index].key == key) {
			// value already in table
			return;
		}
		if(map->data[index].slb == INVALID_HASHMAP_LPN) {
			map->data[index].table = table;
			map->data[index].key = key;
			map->data[index].slb = slb;
			++(map->size);
			return;
		}
	}

	xil_printf("Ran out of space in bucket %d, table %d key %d was hashed to %d.\r\n", bucketId, table, key, hashValue);
	ASSERT(0);
}

unsigned int getHashMap(hashMap* map, unsigned int table, unsigned int key) {
	unsigned int bucketId = hash(table, key) % map->bucketCount;
	int i = 0;
	for(; i != map->bucketLength; ++i) {
		unsigned int index = bucketId * map->bucketLength + i;
		if(map->data[index].key == key) {
			return map->data[index].slb;
		}
	}
	return INVALID_HASHMAP_LPN;
}

hashMap indexMap;
int table1iter = 0;

void InitIndexMap() {
	indexMap = makeHashMap(DEFAULT_BUCKET_LENGTH, DEFAULT_BUCKET_COUNT, MAX_BACKING_SIZE, (void*)HASHMAP_ADDR);
}
