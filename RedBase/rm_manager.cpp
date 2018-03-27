#pragma region Includes
	#include <iostream>

	#include "rm.h"
	#include "rm_internal.h"
#pragma endregion

using namespace std;

RM_Manager::RM_Manager(PF_Manager &pfm) {
	pf_manager = &pfm;
}

RM_Manager::~RM_Manager() {}

RC RM_Manager::CreateFile(const char *fileName, int recordSize) {
	RC WARN = RM_MANAGER_CREATE_WARN, ERR = RM_MANAGER_CREATE_ERR;
	if ((recordSize >= PF_PAGE_SIZE - (int) sizeof(RM_PageHdr)) || (recordSize <= 0)) {
		return RM_BAD_REC_SIZE;
	}
	if (!fileName) return RM_NULL_FILENAME;
	RM_ErrorForward(pf_manager->CreateFile(fileName));
	PF_FileHandle fh;
	PF_PageHandle header;
	RM_ErrorForward(pf_manager->OpenFile(fileName, fh));
	RM_ErrorForward(fh.AllocatePage(header));
	PageNum header_pnum;
	RM_ErrorForward(header.GetPageNum(header_pnum));
	RM_ErrorForward(fh.MarkDirty(header_pnum));
	char *contents;
	RM_ErrorForward(header.GetData(contents));
	RM_FileHdr fHdr;
	fHdr.record_length = recordSize;
	fHdr.capacity = numRecordsPerPage(recordSize);
	fHdr.bitmap_size = fHdr.capacity / 8;
	fHdr.bitmap_offset = sizeof(RM_PageHdr);
	fHdr.first_record_offset = fHdr.bitmap_offset + fHdr.bitmap_size;
	fHdr.header_pnum = header_pnum;
	fHdr.empty_page_count = 0;
	fHdr.first_free = RM_SENTINEL;
	memcpy(contents, &fHdr, sizeof(RM_FileHdr));
	RM_ErrorForward(fh.UnpinPage(header_pnum));
	RM_ErrorForward(pf_manager->CloseFile(fh));
	return OK_RC;
}

RC RM_Manager::DestroyFile(const char *fileName) {
	RC WARN = RM_MANAGER_DESTROY_WARN, ERR = RM_MANAGER_DESTROY_ERR;
	if (!fileName) return RM_NULL_FILENAME;
	RM_ErrorForward(pf_manager->DestroyFile(fileName));
	return OK_RC;
}

RC RM_Manager::OpenFile(const char *fileName, RM_FileHandle &fileHandle) {
	RC WARN = RM_MANAGER_OPEN_WARN, ERR = RM_MANAGER_OPEN_ERR;
	if (!fileName) return RM_NULL_FILENAME;
	if (fileHandle.bIsOpen) {
		RM_ErrorForward(1);
	}
	RM_ErrorForward(pf_manager->OpenFile(fileName, fileHandle.pf_fh));
	PF_PageHandle header;
	RM_ErrorForward(fileHandle.pf_fh.GetFirstPage(header));
	char *data;
	RM_ErrorForward(header.GetData(data));
	memcpy(&fileHandle.fHdr, data, sizeof(RM_FileHdr));
	PageNum header_pnum;
	RM_ErrorForward(header.GetPageNum(header_pnum));
	RM_ErrorForward(fileHandle.pf_fh.UnpinPage(header_pnum));
	fileHandle.bIsOpen = 1;
	fileHandle.bHeaderChanged = 0;
	return OK_RC;
}

RC RM_Manager::CloseFile(RM_FileHandle &fileHandle) {
	RC WARN = RM_MANAGER_CLOSE_WARN, ERR = RM_MANAGER_CLOSE_ERR;

	if (fileHandle.bIsOpen == 0) {
		RM_ErrorForward(1);
	}
	if (fileHandle.bHeaderChanged) {
		PF_PageHandle header;
		int header_pnum = fileHandle.fHdr.header_pnum;
		RM_ErrorForward(fileHandle.pf_fh.GetThisPage(header_pnum, header));
		RM_ErrorForward(fileHandle.pf_fh.MarkDirty(header_pnum));
		char *contents;
		RM_ErrorForward(header.GetData(contents));
		memcpy(contents, &fileHandle.fHdr, sizeof(RM_FileHdr));
		RM_ErrorForward(fileHandle.pf_fh.UnpinPage(header_pnum));
	}
	RM_ErrorForward(fileHandle.ForcePages(ALL_PAGES));
	RM_ErrorForward(pf_manager->CloseFile(fileHandle.pf_fh));
	fileHandle.bIsOpen = 0;
	fileHandle.bHeaderChanged = 0;
	return OK_RC;
}

int RM_Manager::numRecordsPerPage(int rec_size) {
	int num = 0;
	int effective_psize = PF_PAGE_SIZE - sizeof(RM_PageHdr);
	while (num * rec_size + ceil(num / 8.0) <= effective_psize) num++;
	return num - 1;
}