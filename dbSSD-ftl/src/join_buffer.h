#ifndef JOIN_BUFFER_H
#define JOIN_BUFFER_H

unsigned int PageToBuffer(unsigned int slb, unsigned int cmdSlotTag);
void SendJoinPage(unsigned int slb, unsigned int cmdSlotTag, unsigned short length, unsigned short isRemaining);
char* FindRowAddr(unsigned int tableWidth, unsigned int buffEntry, unsigned int key);

#endif
