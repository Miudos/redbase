#pragma region Includes
	#include <iostream>
	#include <io.h>
	#include "pf_buffermgr.h"
	#ifdef STATISTICS_H
		#include "statistics.h"
		StatisticsMgr *pStatisticsMgr;
	#endif // STATISTICS_H
#pragma endregion

#define MEMORY_FD -1

using namespace std;

PF_BufferMgr::PF_BufferMgr(int _numPages) : hashTable(PF_HASH_TBL_SIZE)
{
	this->numPages = _numPages;
	pageSize = PF_PAGE_SIZE + sizeof(PF_PageHdr);

	#ifdef STATISTICS_H
		pStatisticsMgr = new StatisticsMgr();
	#endif

	bufTable = new PF_BufPageDesc[numPages];

	for (int i = 0; i < numPages; i++) {
		if ((bufTable[i].pData = new char[pageSize]) == NULL) {
			cerr << "Not enough memory for buffer\n";
			exit(1);
		}

		memset((void *)bufTable[i].pData, 0, pageSize);

		bufTable[i].prev = i - 1;
		bufTable[i].next = i + 1;
	}
	bufTable[0].prev = bufTable[numPages - 1].next = INVALID_SLOT;
	free = 0;
	first = last = INVALID_SLOT;
}

PF_BufferMgr::~PF_BufferMgr()
{
	for (int i = 0; i < this->numPages; i++)
		delete[] bufTable[i].pData;

	delete[] bufTable;

	#ifdef STATISTICS_H
		delete pStatisticsMgr;
	#endif
}

RC PF_BufferMgr::GetPage(int fd, PageNum pageNum, char **ppBuffer, int bMultiplePins)
{
	RC  rc;
	int slot;
	#ifdef STATISTICS_H
		pStatisticsMgr->Register(PF_GETPAGE, STAT_ADDONE);
	#endif

	if ((rc = hashTable.Find(fd, pageNum, slot)) &&
		(rc != PF_HASHNOTFOUND))
		return (rc);
	if (rc == PF_HASHNOTFOUND) {

	#ifdef STATISTICS_H
		pStatisticsMgr->Register(PF_PAGENOTFOUND, STAT_ADDONE);
	#endif

	if ((rc = InternalAlloc(slot)))
		return (rc);

	if ((rc = ReadPage(fd, pageNum, bufTable[slot].pData)) ||
		(rc = hashTable.Insert(fd, pageNum, slot)) ||
		(rc = InitPageDesc(fd, pageNum, slot))) {		
			Unlink(slot);
			InsertFree(slot);
			return (rc);
		}
	}
	else {
		#ifdef STATISTICS_H
			pStatisticsMgr->Register(PF_PAGEFOUND, STAT_ADDONE);
		#endif

		if (!bMultiplePins && bufTable[slot].pinCount > 0)
			return (PF_PAGEPINNED);
		bufTable[slot].pinCount++;

		if ((rc = Unlink(slot)) ||
			(rc = LinkHead(slot)))
			return (rc);
	}
	*ppBuffer = bufTable[slot].pData;
	return (0);
}

RC PF_BufferMgr::AllocatePage(int fd, PageNum pageNum, char **ppBuffer)
{
	RC  rc;
	int slot;

	if (!(rc = hashTable.Find(fd, pageNum, slot)))
		return (PF_PAGEINBUF);
	else if (rc != PF_HASHNOTFOUND)
		return (rc);
	if ((rc = InternalAlloc(slot)))
		return (rc);

	if ((rc = hashTable.Insert(fd, pageNum, slot)) ||
		(rc = InitPageDesc(fd, pageNum, slot))) {
			Unlink(slot);
			InsertFree(slot);
			return (rc);
		}
	*ppBuffer = bufTable[slot].pData;
	return (0);
}

RC PF_BufferMgr::MarkDirty(int fd, PageNum pageNum)
{
	RC  rc;
	int slot;

	if ((rc = hashTable.Find(fd, pageNum, slot))) {
		if ((rc == PF_HASHNOTFOUND))
			return (PF_PAGENOTINBUF);
		else
			return (rc);
	}

	if (bufTable[slot].pinCount == 0)
		return (PF_PAGEUNPINNED);

	bufTable[slot].bDirty = TRUE;

	if ((rc = Unlink(slot)) ||
		(rc = LinkHead(slot)))
		return (rc);

	return (0);
}

RC PF_BufferMgr::UnpinPage(int fd, PageNum pageNum)
{
	RC  rc;
	int slot;

	if ((rc = hashTable.Find(fd, pageNum, slot))) {
		if ((rc == PF_HASHNOTFOUND))
			return (PF_PAGENOTINBUF);
		else
			return (rc);
	}

	if (bufTable[slot].pinCount == 0)
		return (PF_PAGEUNPINNED);

	if (--(bufTable[slot].pinCount) == 0) {
		if ((rc = Unlink(slot)) ||
			(rc = LinkHead(slot)))
			return (rc);
	}
	return (0);
}

RC PF_BufferMgr::FlushPages(int fd)
{
	RC rc, rcWarn = 0;

	#ifdef STATISTICS_H
		pStatisticsMgr->Register(PF_FLUSHPAGES, STAT_ADDONE);
	#endif

	int slot = first;
	while (slot != INVALID_SLOT) {

		int next = bufTable[slot].next;

		if (bufTable[slot].fd == fd) {

			if (bufTable[slot].pinCount) {
				rcWarn = PF_PAGEPINNED;
			}
			else {
				if (bufTable[slot].bDirty) {
					if ((rc = WritePage(fd, bufTable[slot].pageNum, bufTable[slot].pData)))
						return (rc);
					bufTable[slot].bDirty = FALSE;
				}

				if ((rc = hashTable.Delete(fd, bufTable[slot].pageNum)) ||
					(rc = Unlink(slot)) ||
					(rc = InsertFree(slot)))
					return (rc);
			}
		}
		slot = next;
	}
	return (rcWarn);
}

RC PF_BufferMgr::ForcePages(int fd, PageNum pageNum)
{
	RC rc;

	int slot = first;
	while (slot != INVALID_SLOT) {
		int next = bufTable[slot].next;

		if (bufTable[slot].fd == fd &&
			(pageNum == ALL_PAGES || bufTable[slot].pageNum == pageNum)) {
			if (bufTable[slot].bDirty) {
				if ((rc = WritePage(fd, bufTable[slot].pageNum, bufTable[slot].pData)))
					return (rc);
				bufTable[slot].bDirty = FALSE;
			}
		}
		slot = next;
	}
	return 0;
}

RC PF_BufferMgr::PrintBuffer()
{
	cout << "Buffer contains " << numPages << " pages of size "
		<< pageSize << ".\n";
	cout << "Contents in order from most recently used to "
		<< "least recently used.\n";

	int slot, next;
	slot = first;
	while (slot != INVALID_SLOT) {
		next = bufTable[slot].next;
		cout << slot << " :: \n";
		cout << "  fd = " << bufTable[slot].fd << "\n";
		cout << "  pageNum = " << bufTable[slot].pageNum << "\n";
		cout << "  bDirty = " << bufTable[slot].bDirty << "\n";
		cout << "  pinCount = " << bufTable[slot].pinCount << "\n";
		slot = next;
	}

	if (first == INVALID_SLOT)
		cout << "Buffer is empty!\n";
	else
		cout << "All remaining slots are free.\n";
	return 0;
}

RC PF_BufferMgr::ClearBuffer()
{
	RC rc;

	int slot, next;
	slot = first;
	while (slot != INVALID_SLOT) {
		next = bufTable[slot].next;
		if (bufTable[slot].pinCount == 0)
			if ((rc = hashTable.Delete(bufTable[slot].fd,
				bufTable[slot].pageNum)) ||
				(rc = Unlink(slot)) ||
				(rc = InsertFree(slot)))
				return (rc);
		slot = next;
	}
	return 0;
}

RC PF_BufferMgr::ResizeBuffer(int iNewSize)
{
	int i;
	RC rc;

	ClearBuffer();
	PF_BufPageDesc *pNewBufTable = new PF_BufPageDesc[iNewSize];

	for (i = 0; i < iNewSize; i++) {
		if ((pNewBufTable[i].pData = new char[pageSize]) == NULL) {
			cerr << "Not enough memory for buffer\n";
			exit(1);
		}

		memset((void *)pNewBufTable[i].pData, 0, pageSize);

		pNewBufTable[i].prev = i - 1;
		pNewBufTable[i].next = i + 1;
	}
	pNewBufTable[0].prev = pNewBufTable[iNewSize - 1].next = INVALID_SLOT;

	int oldFirst = first;
	PF_BufPageDesc *pOldBufTable = bufTable;
	numPages = iNewSize;
	first = last = INVALID_SLOT;
	free = 0;

	bufTable = pNewBufTable;
	int slot, next, newSlot;
	slot = oldFirst;
	while (slot != INVALID_SLOT) {
		next = pOldBufTable[slot].next;

		if ((rc = hashTable.Delete(pOldBufTable[slot].fd, pOldBufTable[slot].pageNum)))
			return (rc);
		slot = next;
	}

	slot = oldFirst;
	while (slot != INVALID_SLOT) {
		next = pOldBufTable[slot].next;
		if ((rc = InternalAlloc(newSlot)))
			return (rc);

		if ((rc = hashTable.Insert(pOldBufTable[slot].fd,
			pOldBufTable[slot].pageNum, newSlot)) ||
			(rc = InitPageDesc(pOldBufTable[slot].fd,
				pOldBufTable[slot].pageNum, newSlot)))
			return (rc);

		Unlink(newSlot);
		InsertFree(newSlot);

		slot = next;
	}
	delete[] pOldBufTable;

	return 0;
}

RC PF_BufferMgr::InsertFree(int slot)
{
	bufTable[slot].next = free;
	free = slot;

	return (0);
}

RC PF_BufferMgr::LinkHead(int slot)
{
	bufTable[slot].next = first;
	bufTable[slot].prev = INVALID_SLOT;
	if (first != INVALID_SLOT)
		bufTable[first].prev = slot;

	first = slot;

	if (last == INVALID_SLOT)
		last = first;

	return (0);
}

RC PF_BufferMgr::Unlink(int slot)
{
	if (first == slot)
		first = bufTable[slot].next;
	if (last == slot)
		last = bufTable[slot].prev;
	if (bufTable[slot].next != INVALID_SLOT)
		bufTable[bufTable[slot].next].prev = bufTable[slot].prev;
	if (bufTable[slot].prev != INVALID_SLOT)
		bufTable[bufTable[slot].prev].next = bufTable[slot].next;

	bufTable[slot].prev = bufTable[slot].next = INVALID_SLOT;

	return (0);
}

RC PF_BufferMgr::InternalAlloc(int &slot)
{
	RC  rc;

	if (free != INVALID_SLOT) {
		slot = free;
		free = bufTable[slot].next;
	}
	else {
		for (slot = last; slot != INVALID_SLOT; slot = bufTable[slot].prev) {
			if (bufTable[slot].pinCount == 0)
				break;
		}
		if (slot == INVALID_SLOT)
			return (PF_NOBUF);

		if (bufTable[slot].bDirty) {
			if ((rc = WritePage(bufTable[slot].fd, bufTable[slot].pageNum,
				bufTable[slot].pData)))
				return (rc);

			bufTable[slot].bDirty = FALSE;
		}
		if ((rc = hashTable.Delete(bufTable[slot].fd, bufTable[slot].pageNum)) ||
			(rc = Unlink(slot)))
			return (rc);
	}

	if ((rc = LinkHead(slot)))
		return (rc);
	return (0);
}

RC PF_BufferMgr::ReadPage(int fd, PageNum pageNum, char *dest)
{
	#ifdef STATISTICS_H
		pStatisticsMgr->Register(PF_READPAGE, STAT_ADDONE);
	#endif

	long offset = pageNum * (long)pageSize + PF_FILE_HDR_SIZE;
	if (_lseek(fd, offset, L_SET) < 0)
		return (PF_UNIX);

	int numBytes = _read(fd, dest, pageSize);
	if (numBytes < 0)
		return (PF_UNIX);
	else if (numBytes != pageSize)
		return (PF_INCOMPLETEREAD);
	else
		return (0);
}

RC PF_BufferMgr::WritePage(int fd, PageNum pageNum, char *source)
{
	#ifdef STATISTICS_H
		pStatisticsMgr->Register(PF_WRITEPAGE, STAT_ADDONE);
	#endif

	long offset = pageNum * (long)pageSize + PF_FILE_HDR_SIZE;
	if (_lseek(fd, offset, L_SET) < 0)
		return (PF_UNIX);

	int numBytes = _write(fd, source, pageSize);
	if (numBytes < 0)
		return (PF_UNIX);
	else if (numBytes != pageSize)
		return (PF_INCOMPLETEWRITE);
	else
		return (0);
}

RC PF_BufferMgr::InitPageDesc(int fd, PageNum pageNum, int slot)
{
	bufTable[slot].fd = fd;
	bufTable[slot].pageNum = pageNum;
	bufTable[slot].bDirty = FALSE;
	bufTable[slot].pinCount = 1;

	return (0);
}

RC PF_BufferMgr::GetBlockSize(int &length) const
{
	length = pageSize;
	return OK_RC;
}

RC PF_BufferMgr::AllocateBlock(char *&buffer)
{
	RC rc = OK_RC;

	int slot;
	if ((rc = InternalAlloc(slot)) != OK_RC)
		return rc;

	PageNum pageNum = bufTable[slot].pData - (char*)0;

	if ((rc = hashTable.Insert(MEMORY_FD, pageNum, slot) != OK_RC) ||
		(rc = InitPageDesc(MEMORY_FD, pageNum, slot)) != OK_RC) {
		Unlink(slot);
		InsertFree(slot);
		return rc;
	}

	buffer = bufTable[slot].pData;
	return OK_RC;
}

RC PF_BufferMgr::DisposeBlock(char* buffer)
{
	return UnpinPage(MEMORY_FD, buffer - (char*)0);
}