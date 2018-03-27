#pragma once
#ifndef PF_BUFFERMNGR_H
#define PF_BUFFERMNGR_H

	#pragma region Includes
		#include "pf_internal.h"
		#include "pf_hashtable.h"
	#pragma endregion

#define INVALID_SLOT (-1)

struct PF_BufPageDesc {
	char *pData;
	int next;
	int prev;
	int bDirty;
	short int pinCount;
	PageNum pageNum;
	int fd;
};

class PF_BufferMgr {
public:
	PF_BufferMgr(int numPages);
	~PF_BufferMgr();
	RC  GetPage(int fd, PageNum pageNum, char **ppBuffer,int bMultiplePins = TRUE);
	RC  AllocatePage(int fd, PageNum pageNum, char **ppBuffer);
	RC  MarkDirty(int fd, PageNum pageNum);
	RC  UnpinPage(int fd, PageNum pageNum);
	RC  FlushPages(int fd);
	RC ForcePages(int fd, PageNum pageNum);
	RC  ClearBuffer();
	RC PrintBuffer();
	RC ResizeBuffer(int iNewSize);
	RC GetBlockSize(int &length) const;
	RC AllocateBlock(char *&buffer);
	RC DisposeBlock(char *buffer);
private:
	RC  InsertFree(int slot);
	RC  LinkHead(int slot);
	RC  Unlink(int slot);
	RC  InternalAlloc(int &slot);
	RC  ReadPage(int fd, PageNum pageNum, char *dest);
	RC  WritePage(int fd, PageNum pageNum, char *source);
	RC  InitPageDesc(int fd, PageNum pageNum, int slot);

	PF_BufPageDesc *bufTable;
	PF_HashTable hashTable;
	int numPages;
	int pageSize;
	int first;
	int last;
	int free;
};

#endif // !PF_BUFFERMNGR_H
