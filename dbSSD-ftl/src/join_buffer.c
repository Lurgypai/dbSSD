#include "join_buffer.h"
#include "memory_map.h"
#include "debug.h"
#include "low_level_scheduler.h"
#include "lru_buffer.h"

unsigned int PageToBuffer(unsigned int slb, unsigned int cmdSlotTag) {
	unsigned int tempLpn, dieNo, dieLpn;

	tempLpn = slb / SECTOR_NUM_PER_PAGE;
	dieNo = tempLpn % DIE_NUM;
	dieLpn = tempLpn / DIE_NUM;

	unsigned int bufferEntry = CheckBufHit(tempLpn);
	if(bufferEntry != 0x7fff) {
		return bufferEntry;
	}
	else
	{
		bufferEntry = AllocateBufEntry(tempLpn);

		bufMap->bufEntry[bufferEntry].dirty = 0;

		//link
		dieNo = tempLpn % DIE_NUM;
		if(bufLruList->bufLruEntry[dieNo].head != 0x7fff)
		{
			bufMap->bufEntry[bufferEntry].prevEntry = 0x7fff;
			bufMap->bufEntry[bufferEntry].nextEntry = bufLruList->bufLruEntry[dieNo].head;
			bufMap->bufEntry[bufLruList->bufLruEntry[dieNo].head].prevEntry = bufferEntry;
			bufLruList->bufLruEntry[dieNo].head = bufferEntry;
		}
		else
		{
			bufMap->bufEntry[bufferEntry].prevEntry = 0x7fff;
			bufMap->bufEntry[bufferEntry].nextEntry = 0x7fff;
			bufLruList->bufLruEntry[dieNo].head = bufferEntry;
			bufLruList->bufLruEntry[dieNo].tail = bufferEntry;
		}
		bufMap->bufEntry[bufferEntry].lpn = tempLpn;


		LOW_LEVEL_REQ_INFO lowLevelCmd;

		if (pageMap->pmEntry[dieNo][dieLpn].ppn != 0xffffffff)
		{
			lowLevelCmd.rowAddr = pageMap->pmEntry[dieNo][dieLpn].ppn;
			lowLevelCmd.spareDataBuf = SPARE_ADDR;
			lowLevelCmd.devAddr = (BUFFER_ADDR + bufferEntry * BUF_ENTRY_SIZE);
			lowLevelCmd.cmdSlotTag = cmdSlotTag;
			lowLevelCmd.startDmaIndex = 0;
			lowLevelCmd.chNo = dieNo % CHANNEL_NUM;
			lowLevelCmd.wayNo = dieNo / CHANNEL_NUM;
			lowLevelCmd.subReqSect = SECTOR_NUM_PER_PAGE;
			lowLevelCmd.bufferEntry = bufferEntry;
			lowLevelCmd.request = V2FCommand_ReadPageTrigger;
			PushToReqQueue(&lowLevelCmd);
		}
		else
		{
			xil_printf("Unable to find page associated with slb %d\r\n", slb);
		}

		reservedReq = 1;

		return bufferEntry;
	}
}

void SendJoinPage(unsigned int slb, unsigned int cmdSlotTag, unsigned short length, unsigned short isRemainingPages) {

	if(length > PAGE_SIZE - 2) {
		xil_printf("Value for join page length to pass is irregularly large: %d\r\n", length);
		ASSERT(0);
	}

	//xil_printf("Length of join page: %d, isRemainingPages: %d\r\n", length, isRemainingPages);


	// create and set the 2 flag bytes
	unsigned short flagBytes = length | (isRemainingPages << 14);

	*((unsigned short*)(JOIN_PAGE_ADDR + PAGE_SIZE - 2)) = flagBytes;

	LOW_LEVEL_REQ_INFO lowLevelCmd;
	unsigned int tempLpn = slb / SECTOR_NUM_PER_PAGE;
	unsigned int dieNo = tempLpn % DIE_NUM;

	lowLevelCmd.devAddr = JOIN_PAGE_ADDR;
	lowLevelCmd.cmdSlotTag = cmdSlotTag;
	lowLevelCmd.startDmaIndex = 0;
	lowLevelCmd.chNo = dieNo % CHANNEL_NUM;
	lowLevelCmd.wayNo = dieNo / CHANNEL_NUM;
	lowLevelCmd.subReqSect = SECTOR_NUM_PER_PAGE;
	lowLevelCmd.bufferEntry = BUF_ENTRY_NUM; // added an extra for this
	lowLevelCmd.request = LLSCommand_TxDMA;

	PushToReqQueue(&lowLevelCmd);
	reservedReq = 1;
}


char* FindRowAddr(unsigned int tableWidth, unsigned int buffEntry, unsigned int key) {
	int row;
	for(row = 0; row != PAGE_SIZE / tableWidth; ++row) {
		unsigned int offset = row * tableWidth;
		unsigned int addr = (BUFFER_ADDR + buffEntry * BUF_ENTRY_SIZE + offset);
		unsigned long long rowKey = *((unsigned long long*)addr);
		if(rowKey == key) {
			return (char*)addr;
		}
	}
	return NULL;
}
