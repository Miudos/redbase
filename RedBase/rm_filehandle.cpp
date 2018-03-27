#pragma region Includes
	#include <iostream>

	#include "rm.h"
	#include "rm_internal.h"
#pragma endregion

using namespace std;

RM_FileHandle::RM_FileHandle() {
	bIsOpen = 0;
	bHeaderChanged = 0;
}

RM_FileHandle::~RM_FileHandle() {}

RC RM_FileHandle::GetRec(const RID &rid, RM_Record &rec) const {
	RC WARN = RM_INVALID_RID, ERR = RM_FILEHANDLE_FATAL; // used by macro
	if (bIsOpen == 0) return RM_FILE_NOT_OPEN;
	int pnum;
	int snum;
	RM_ErrorForward(rid.GetPageNum(pnum));
	RM_ErrorForward(rid.GetSlotNum(snum));
	PF_PageHandle page;
	RM_ErrorForward(pf_fh.GetThisPage(pnum, page));
	char *data;
	RM_ErrorForward(page.GetData(data));
	int rec_exists;
	RM_ErrorForward(GetBit(data + fHdr.bitmap_offset, snum, rec_exists));
	if (rec_exists == 0) RM_ErrorForward(1);
	if (rec.bIsAllocated) delete[] rec.record;
	rec.record = new char[fHdr.record_length];
	RM_ErrorForward(FetchRecord(data, rec.record, snum));
	rec.bIsAllocated = 1;
	rec.rid = rid;
	RM_ErrorForward(pf_fh.UnpinPage(pnum));
	return OK_RC;
}

RC RM_FileHandle::InsertRec(const char *pData, RID &rid) {
	RC WARN = RM_INSERT_FAIL, ERR = RM_FILEHANDLE_FATAL; // used by macro
	if (bIsOpen == 0) return RM_FILE_NOT_OPEN;
	if (!pData) return RM_NULL_INSERT;
	PF_PageHandle pf_ph;
	int dest_page;
	if (fHdr.first_free == RM_SENTINEL) {
		pf_fh.AllocatePage(pf_ph);
		pf_ph.GetPageNum(dest_page);
	}
	else {
		dest_page = fHdr.first_free;
		pf_fh.GetThisPage(dest_page, pf_ph);
	}	char* data;
	RM_ErrorForward(pf_ph.GetData(data));
	RM_ErrorForward(pf_fh.MarkDirty(dest_page));
	if (fHdr.first_free == RM_SENTINEL) {
		fHdr.first_free = dest_page;
		((RM_PageHdr*)data)->next_free = RM_SENTINEL;
		bHeaderChanged = 1;
	}
	int dest_slot = FindSlot(data + fHdr.bitmap_offset);
	RM_ErrorForward(DumpRecord(data, pData, dest_slot));
	((RM_PageHdr*)data)->num_recs++;
	RM_ErrorForward(SetBit(data + fHdr.bitmap_offset, dest_slot));
	if (((RM_PageHdr*)data)->num_recs == fHdr.capacity) {
		fHdr.first_free = ((RM_PageHdr*)data)->next_free;
		bHeaderChanged = 1;
	}
	rid = RID(dest_page, dest_slot);
	RM_ErrorForward(pf_fh.UnpinPage(dest_page));
	return OK_RC;
}

RC RM_FileHandle::DeleteRec(const RID &rid) {
	RC WARN = RM_INVALID_RID, ERR = RM_FILEHANDLE_FATAL;
	if (bIsOpen == 0) return RM_FILE_NOT_OPEN;
	int pnum;
	int snum;
	RM_ErrorForward(rid.GetPageNum(pnum));
	RM_ErrorForward(rid.GetSlotNum(snum));
	PF_PageHandle page;
	RM_ErrorForward(pf_fh.GetThisPage(pnum, page));
	char *data;
	RM_ErrorForward(page.GetData(data));
	int rec_exists;
	RM_ErrorForward(GetBit(data + fHdr.bitmap_offset, snum, rec_exists));
	if (rec_exists == 0) {
		RM_ErrorForward(pf_fh.UnpinPage(pnum));
		RM_ErrorForward(1);
	}
	RM_ErrorForward(pf_fh.MarkDirty(pnum));
	RM_ErrorForward(UnsetBit(data + fHdr.bitmap_offset, snum));
	if (((RM_PageHdr*)data)->num_recs == fHdr.capacity) {
		((RM_PageHdr*)data)->next_free = fHdr.first_free;
		fHdr.first_free = pnum;
		bHeaderChanged = 1;
	}
	((RM_PageHdr*)data)->num_recs--;
	RM_ErrorForward(pf_fh.UnpinPage(pnum));
	return OK_RC;
}

RC RM_FileHandle::UpdateRec(const RM_Record &rec) {
	RC WARN = RM_INVALID_RECORD, ERR = RM_FILEHANDLE_FATAL;
	if (bIsOpen == 0) return RM_FILE_NOT_OPEN;
	if (rec.bIsAllocated == 0) return RM_INVALID_RECORD;
	int pnum;
	int snum;
	RM_ErrorForward(rec.rid.GetPageNum(pnum));
	RM_ErrorForward(rec.rid.GetSlotNum(snum));
	PF_PageHandle page;
	RM_ErrorForward(pf_fh.GetThisPage(pnum, page));
	char *data;
	RM_ErrorForward(page.GetData(data));
	int rec_exists;
	RM_ErrorForward(GetBit(data + fHdr.bitmap_offset, snum, rec_exists));
	if (rec_exists == 0) RM_ErrorForward(1);

	RM_ErrorForward(pf_fh.MarkDirty(pnum));
	RM_ErrorForward(DumpRecord(data, rec.record, snum));
	RM_ErrorForward(pf_fh.UnpinPage(pnum));
	return OK_RC;
}

RC RM_FileHandle::ForcePages(int pageNum) const {
	RC WARN = RM_FORCEPAGE_FAIL, ERR = RM_FILEHANDLE_FATAL;
	if (bIsOpen == 0) return RM_FILE_NOT_OPEN;
	if (bHeaderChanged) {
		PF_PageHandle header;
		int header_pnum = fHdr.header_pnum;
		RM_ErrorForward(pf_fh.GetThisPage(header_pnum, header));
		RM_ErrorForward(pf_fh.MarkDirty(header_pnum));
		char *contents;
		RM_ErrorForward(header.GetData(contents));
		memcpy(contents, &fHdr, sizeof(RM_FileHdr));
		RM_ErrorForward(pf_fh.UnpinPage(header_pnum));
	}
	if (pageNum == ALL_PAGES) {
		RM_ErrorForward(pf_fh.FlushPages());
	}
	else {
		RM_ErrorForward(pf_fh.ForcePages(pageNum));
	}
	return OK_RC;
}

RC RM_FileHandle::FetchRecord(char *page, char *buffer, int slot) const {
	if (slot >= fHdr.capacity) return RM_INVALID_RID;
	char *location = page + fHdr.first_record_offset
		+ slot * fHdr.record_length;
	memcpy(buffer, location, fHdr.record_length);
	return OK_RC;
}

RC RM_FileHandle::DumpRecord(char *page, const char *buffer, int slot) {
	if (slot >= fHdr.capacity) return RM_INVALID_RID;
	char *location = page + fHdr.first_record_offset
		+ slot * fHdr.record_length;
	memcpy(location, buffer, fHdr.record_length);
	return OK_RC;
}

RC RM_FileHandle::SetBit(char *bitmap, int index) const {
	if (index >= fHdr.capacity) return RM_PAGE_OVERFLOW;
	int byte = index / 8;
	int b_ind = index % 8;
	*(bitmap + byte) |= (1 << (7 - b_ind));
	return OK_RC;
}

RC RM_FileHandle::UnsetBit(char *bitmap, int index) const {
	if (index >= fHdr.capacity) return RM_PAGE_OVERFLOW;
	int byte = index / 8;
	int b_ind = index % 8;
	*(bitmap + byte) &= (~(1 << (7 - b_ind)));
	return OK_RC;
}

RC RM_FileHandle::GetBit(char *bitmap, int index, int& status) const {
	if (index >= fHdr.capacity) return RM_PAGE_OVERFLOW;
	int byte = index / 8;
	int b_ind = index % 8;
	status = (*(bitmap + byte) & (1 << (7 - b_ind))) > 0;
	return OK_RC;
}

int RM_FileHandle::FindSlot(char *bitmap) const {
	int slot = -1, base = 0;
	unsigned char byte;
	for (int i = 0; i < fHdr.bitmap_size; i++) {
		byte = (unsigned char)~*(bitmap + i);
		if (byte == 0) {
			base += 8;
			continue;
		}
		slot = base + 7 - floor(log2(byte));
		break;
	}
	if (slot >= fHdr.capacity) slot = -1;
	return slot;
}
