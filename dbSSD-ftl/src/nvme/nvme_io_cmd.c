//////////////////////////////////////////////////////////////////////////////////
// nvme_io_cmd.c for Cosmos+ OpenSSD
// Copyright (c) 2016 Hanyang University ENC Lab.
// Contributed by Yong Ho Song <yhsong@enc.hanyang.ac.kr>
//				  Youngjin Jo <yjjo@enc.hanyang.ac.kr>
//				  Sangjin Lee <sjlee@enc.hanyang.ac.kr>
//				  Jaewook Kwak <jwkwak@enc.hanyang.ac.kr>
//
// This file is part of Cosmos+ OpenSSD.
//
// Cosmos+ OpenSSD is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3, or (at your option)
// any later version.
//
// Cosmos+ OpenSSD is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Cosmos+ OpenSSD; see the file COPYING.
// If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
// Company: ENC Lab. <http://enc.hanyang.ac.kr>
// Engineer: Sangjin Lee <sjlee@enc.hanyang.ac.kr>
//			 Jaewook Kwak <jwkwak@enc.hanyang.ac.kr>
//
// Project Name: Cosmos+ OpenSSD
// Design Name: Cosmos+ Firmware
// Module Name: NVMe IO Command Handler
// File Name: nvme_io_cmd.c
//
// Version: v1.0.1
//
// Description:
//   - handles NVMe IO command
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
// Revision History:
//
// * v1.0.1
//   - header file for buffer is changed from "ia_lru_buffer.h" to "lru_buffer.h"
//
// * v1.0.0
//   - First draft
//////////////////////////////////////////////////////////////////////////////////


#include "xil_printf.h"
#include "debug.h"
#include "io_access.h"

#include "nvme.h"
#include "host_lld.h"
#include "nvme_io_cmd.h"
#include "../hashmap.h"
#include "../table.h"

#include "../lru_buffer.h"
#include "../join_buffer.h"

#include "../low_level_scheduler.h"
#include "../memory_map.h"

void handle_nvme_io_read(unsigned int cmdSlotTag, NVME_IO_COMMAND *nvmeIOCmd)
{
	IO_READ_COMMAND_DW12 readInfo12;
	//IO_READ_COMMAND_DW13 readInfo13;
	//IO_READ_COMMAND_DW15 readInfo15;
	unsigned int startLba[2];
	unsigned int nlb;


	readInfo12.dword = nvmeIOCmd->dword[12];
	//readInfo13.dword = nvmeIOCmd->dword[13];
	//readInfo15.dword = nvmeIOCmd->dword[15];

	startLba[0] = nvmeIOCmd->dword[10];
	startLba[1] = nvmeIOCmd->dword[11];
	nlb = readInfo12.NLB;

	ASSERT(startLba[0] < storageCapacity_L && (startLba[1] < STORAGE_CAPACITY_H || startLba[1] == 0));
	//ASSERT(nlb < MAX_NUM_OF_NLB);
	ASSERT((nvmeIOCmd->PRP1[0] & 0xF) == 0 && (nvmeIOCmd->PRP2[0] & 0xF) == 0); //error
	ASSERT(nvmeIOCmd->PRP1[1] < 0x10 && nvmeIOCmd->PRP2[1] < 0x10);

	HOST_REQ_INFO hostCmd;
	hostCmd.curSect = startLba[0];
	hostCmd.reqSect = nlb + 1;
	hostCmd.cmdSlotTag = cmdSlotTag;

	LRUBufRead(&hostCmd);
}


void handle_nvme_io_write(unsigned int cmdSlotTag, NVME_IO_COMMAND *nvmeIOCmd)
{
	IO_READ_COMMAND_DW12 writeInfo12;
	//IO_READ_COMMAND_DW13 writeInfo13;
	//IO_READ_COMMAND_DW15 writeInfo15;
	unsigned int startLba[2];
	unsigned int nlb;

	writeInfo12.dword = nvmeIOCmd->dword[12];
	//writeInfo13.dword = nvmeIOCmd->dword[13];
	//writeInfo15.dword = nvmeIOCmd->dword[15];

	if(writeInfo12.FUA == 1)
		xil_printf("write FUA\r\n");

	startLba[0] = nvmeIOCmd->dword[10];
	startLba[1] = nvmeIOCmd->dword[11];
	nlb = writeInfo12.NLB;

	ASSERT(startLba[0] < storageCapacity_L && (startLba[1] < STORAGE_CAPACITY_H || startLba[1] == 0));
	//ASSERT(nlb < MAX_NUM_OF_NLB);
	ASSERT((nvmeIOCmd->PRP1[0] & 0xF) == 0 && (nvmeIOCmd->PRP2[0] & 0xF) == 0);
	ASSERT(nvmeIOCmd->PRP1[1] < 0x10 && nvmeIOCmd->PRP2[1] < 0x10);

	HOST_REQ_INFO hostCmd;
	hostCmd.curSect = startLba[0];
	hostCmd.reqSect = nlb + 1;
	hostCmd.cmdSlotTag = cmdSlotTag;

	LRUBufWrite(&hostCmd);
}

void handle_nvme_io_cmd(NVME_COMMAND *nvmeCmd)
{
	NVME_IO_COMMAND *nvmeIOCmd;
	NVME_COMPLETION nvmeCPL;
	unsigned int opc;

	nvmeIOCmd = (NVME_IO_COMMAND*)nvmeCmd->cmdDword;
	opc = (unsigned int)nvmeIOCmd->OPC;

	switch(opc)
	{
		case 8:
		{
			xil_printf("Received write 0's, acknowledging as success...\r\n");
			nvmeCPL.dword[0] = 0;
			nvmeCPL.specific = 0x0;
			set_auto_nvme_cpl(nvmeCmd->cmdSlotTag, nvmeCPL.specific, nvmeCPL.statusFieldWord);

		}
		case IO_NVM_FLUSH:
		{
			xil_printf("IO Flush Command\r\n");
			nvmeCPL.dword[0] = 0;
			nvmeCPL.specific = 0x0;
			set_auto_nvme_cpl(nvmeCmd->cmdSlotTag, nvmeCPL.specific, nvmeCPL.statusFieldWord);
			break;
		}
		case IO_NVM_WRITE:
		{
//			xil_printf("IO Write Command\r\n");
			handle_nvme_io_write(nvmeCmd->cmdSlotTag, nvmeIOCmd);
			break;
		}
		case IO_DEBUG_CUSTOM_READ:
		{
			xil_printf("TEST Read Command\r\n");
			handle_nvme_io_read(nvmeCmd->cmdSlotTag, nvmeIOCmd);
			break;
		}
		case IO_NVM_READ:
		{
//			xil_printf("IO Read Command\r\n");
			handle_nvme_io_read(nvmeCmd->cmdSlotTag, nvmeIOCmd);
			break;
		}
		case IO_DB_PRINTMAP:
		{
			int i = 0;
			int bucket = 0;
			int bucketIndex = 0;

			xil_printf("indexMap:\r\n");
			xil_printf("indexMap.bucketLength: %d\r\n", indexMap.bucketLength);
			xil_printf("indexMap.bucketCount: %d\r\n", indexMap.bucketCount);
			xil_printf("indexMap.size: %d\r\n", indexMap.size);
			xil_printf("indexMap.backingSize: %d\r\n", indexMap.backingSize);
			for(; i != indexMap.bucketCount; ++i) {
				int index = bucket * indexMap.bucketLength + bucketIndex;
				if(indexMap.data[index].slb != INVALID_HASHMAP_LPN) {
					xil_printf("    index: %d, Table: %d, Key: %d, SLB: %d\r\n", index, indexMap.data[index].table, indexMap.data[index].key, indexMap.data[index].slb);
					++bucketIndex; //move to next bucket if this one was valid
					if(bucketIndex == indexMap.bucketLength) {
						bucketIndex = 0;
						++bucket;
					}
				} else {
					bucketIndex = 0;
					++bucket;
				}
			}

			nvmeCPL.dword[0] = 0;
			nvmeCPL.specific = 0x0;
			set_auto_nvme_cpl(nvmeCmd->cmdSlotTag, nvmeCPL.specific, nvmeCPL.statusFieldWord);
			break;
		}
		case IO_DB_INSERT:
		{
			unsigned int table = nvmeCmd->cmdDword[13];
			unsigned int key = nvmeCmd->cmdDword[14];
			unsigned int slb = nvmeCmd->cmdDword[10];
			//xil_printf("Inserting into Table: %d, Key: %d, SLB: %d\r\n", table, key, slb);
			insertHashMap(&indexMap, table, key, slb);
			//xil_printf("Running write.\r\n");
			handle_nvme_io_write(nvmeCmd->cmdSlotTag, nvmeIOCmd);
			break;
		}
		case IO_DB_JOIN:
		{
			unsigned int table1 = nvmeCmd->cmdDword[13];
			unsigned int table2 = nvmeCmd->cmdDword[14];

			//xil_printf("Attempting join on tables %d, %d, starting at iter %d\r\n", table1, table2, table1iter);

			int filledPage = 0;
			unsigned int joinPageOffset = 0;
			for(; table1iter != indexMap.bucketCount * indexMap.bucketLength; ++table1iter) {
				// if the first iterator is at an instance of table 1

				unsigned int currentPage1Slb = indexMap.data[table1iter].slb;
				if(currentPage1Slb != INVALID_HASHMAP_LPN && indexMap.data[table1iter].table == table1) {
					unsigned int key = indexMap.data[table1iter].key;
					// xil_printf("Found key %d in table %d\r\n", key, table1);

					unsigned int currentPage2Slb = getHashMap(&indexMap, table2, key);
					if(currentPage2Slb != INVALID_HASHMAP_LPN) {
						// xil_printf("Found key %d in table %d\r\n", key, table2);

						unsigned int page1BufferEntry, page2BufferEntry;
						page1BufferEntry = PageToBuffer(currentPage1Slb, nvmeCmd->cmdSlotTag);
						page2BufferEntry = PageToBuffer(currentPage2Slb, nvmeCmd->cmdSlotTag);

						// xil_printf("Executing page retrieval requests...\r\n");
						EmptyReqQ();

						// xil_printf("Beginning join on tables %d, %d, key %d...\r\n", table1, table2, key);


						int table1width = tables[table1].rowWidth;
						int table2width = tables[table2].rowWidth;

						// find starting rows
						char* table1source = FindRowAddr(table1width, page1BufferEntry, key);
						char* table2source = FindRowAddr(table2width, page2BufferEntry, key);

						char* targetAddr = (char*)(JOIN_PAGE_ADDR + joinPageOffset);
						if(table1source && table2source) {
							// xil_printf("Found target row with key %d in both pages...\r\n", key);
							int i;
							for(i = 0; i != table1width; ++i) {
								targetAddr[i] = table1source[i];
							}
							// skip the key in the second row
							for(i = 0; i != table2width - sizeof(unsigned long long); ++i) {
								targetAddr[i + table1width] = table2source[i + sizeof(unsigned long long)];
							}
						}

						joinPageOffset += table1width + table2width - sizeof(unsigned long long);

						// the minus 2 gives us a status short
						if(joinPageOffset + table1width + table2width - sizeof(unsigned long long) > PAGE_SIZE - 2) {
							// xil_printf("Filled join page, returning to host\r\n");
							SendJoinPage(currentPage1Slb, nvmeCmd->cmdSlotTag, joinPageOffset, 1);
							filledPage = 1;
							++table1iter;
							break;
						}
					}
				}
			}

			if(!filledPage) {
				//xil_printf("Completed join, returning to host\r\n");

				SendJoinPage(indexMap.data[table1iter].slb, nvmeCmd->cmdSlotTag, joinPageOffset, 0);
			}

			//xil_printf("Join completed\r\n");
			break;
		}
		case IO_DB_INSERT_TABLE:
		{
			unsigned int tableId = nvmeCmd->cmdDword[10];
			unsigned int rowWidth = nvmeCmd->cmdDword[11];

			//xil_printf("Adding table with id %d, rowWidth %d.\r\n", tableId, rowWidth);

			insertTable(tableId, rowWidth);

			//xil_printf("Table inserted.\r\n");

			nvmeCPL.dword[0] = 0;
			nvmeCPL.specific = 0x0;
			set_auto_nvme_cpl(nvmeCmd->cmdSlotTag, nvmeCPL.specific, nvmeCPL.statusFieldWord);
			break;
		}
		case IO_DB_PRINT_TABLES:
		{
			xil_printf("Printing tables...\r\n");

			int i = 0;
			for(; i != TABLE_COUNT; ++i) {
				xil_printf("    Table id: %d, rowWidth %d\r\n", tables[i].id, tables[i].rowWidth);
			}

			nvmeCPL.dword[0] = 0;
			nvmeCPL.specific = 0x0;
			set_auto_nvme_cpl(nvmeCmd->cmdSlotTag, nvmeCPL.specific, nvmeCPL.statusFieldWord);
			break;
		}
		case IO_DB_RESET_TABLE_ITERS:
		{
			//xil_printf("Resetting table iter value from %d...\r\n", table1iter);
			table1iter = 0;
			//xil_printf("Table iter reset.\r\n");

			nvmeCPL.dword[0] = 0;
			nvmeCPL.specific = 0x0;
			set_auto_nvme_cpl(nvmeCmd->cmdSlotTag, nvmeCPL.specific, nvmeCPL.statusFieldWord);
			break;
		}
		default:
		{
			xil_printf("Not Support IO Command OPC: %X\r\n", opc);
			ASSERT(0);
			break;
		}
	}
}

