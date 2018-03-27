#pragma once

#ifndef RM_H
	#define RM_H

	#pragma region Includes
		#include "redbase.h"
		#include "rm_rid.h"
		#include "pf.h"
	#pragma endregion

	#pragma region Defines

		#define RM_BAD_REC_SIZE             (START_RM_WARN + 0)
		#define RM_MANAGER_CREATE_WARN      (START_RM_WARN + 1)
		#define RM_MANAGER_DESTROY_WARN     (START_RM_WARN + 2)
		#define RM_MANAGER_OPEN_WARN        (START_RM_WARN + 3)
		#define RM_MANAGER_CLOSE_WARN       (START_RM_WARN + 4)
		#define RM_FILE_NOT_OPEN            (START_RM_WARN + 5)
		#define RM_INVALID_RID              (START_RM_WARN + 6)
		#define RM_INVALID_RECORD           (START_RM_WARN + 7)        
		#define RM_INVALID_PAGE             (START_RM_WARN + 8)        
		#define RM_INSERT_FAIL              (START_RM_WARN + 9)
		#define RM_FORCEPAGE_FAIL           (START_RM_WARN + 10)    
		#define RM_SCAN_NOT_OPEN            (START_RM_WARN + 11)    
		#define RM_PAGE_OVERFLOW            (START_RM_WARN + 12)
		#define RM_NULL_INSERT              (START_RM_WARN + 13)
		#define RM_SCAN_OPEN_FAIL           (START_RM_WARN + 14)
		#define RM_NULL_FILENAME            (START_RM_WARN + 15)
		#define RM_EOF                      (START_RM_WARN + 16)        
		#define RM_LASTWARN                 RM_EOF

		#define RM_MANAGER_CREATE_ERR       (START_RM_ERR - 0)
		#define RM_MANAGER_DESTROY_ERR      (START_RM_ERR - 1)
		#define RM_MANAGER_OPEN_ERR         (START_RM_ERR - 2)
		#define RM_MANAGER_CLOSE_ERR        (START_RM_ERR - 3)
		#define RM_FILEHANDLE_FATAL         (START_RM_ERR - 4)      
		#define RM_FILESCAN_FATAL           (START_RM_ERR - 5)      
		#define RM_LASTERROR                RM_FILESCAN_FATAL
	#pragma endregion

	void RM_PrintError(RC rc);

	struct RM_FileHdr {
		int record_length;
		int capacity;
		int bitmap_size;
		int bitmap_offset;
		int first_record_offset;
		int empty_page_count;
		int header_pnum;
		int first_free;
	};

	class RM_Record {
		friend class RM_FileHandle;
		friend class RM_FileScan;
		friend class QL_Manager;
	public:
		RM_Record();
		~RM_Record();
		RC GetData(char *&pData) const;

		RC GetRid(RID &rid) const;
	private:
		RID rid;
		char *record;
		int bIsAllocated;
	};

	class RM_FileHandle {
		friend class RM_Manager;
		friend class RM_FileScan;
	public:
		RM_FileHandle();
		~RM_FileHandle();

		RC GetRec(const RID &rid, RM_Record &rec) const;
		RC InsertRec(const char *pData, RID &rid);
		RC DeleteRec(const RID &rid);
		RC UpdateRec(const RM_Record &rec);

		RC ForcePages(int pageNum) const;
	private:
		RM_FileHdr fHdr;
		PF_FileHandle pf_fh;
		int bIsOpen;
		int bHeaderChanged;
		RC SetBit(char *bitarray, int position) const;
		RC UnsetBit(char *bitarray, int position) const;
		RC GetBit(char *bitarray, int position, int &status) const;
		int FindSlot(char *bitmap) const;
		RC FetchRecord(char *page, char *buffer, int slot) const;
		RC DumpRecord(char *page, const char *buffer, int slot);
	};

	class RM_FileScan {
	public:
		RM_FileScan();
		~RM_FileScan();

		RC OpenScan(const RM_FileHandle &fileHandle,
			AttrType attrType,
			int attrLength,
			int attrOffset,
			CompOp compOp,
			void *value,
			ClientHint pinHint = NO_HINT);

		RC GetNextRec(RM_Record &rec);
		RC CloseScan();
	private:
		const RM_FileHandle *rm_fh;
		int bIsOpen;
		int attr_offset;
		int attr_length;
		AttrType attr_type;
		char *query_value;
		ClientHint pin_hint;
		CompOp comp_op;
		int recs_seen;
		int num_recs;
		int current;
		char *bitmap_copy;
		PF_PageHandle pf_ph;
		bool (RM_FileScan::*comp)(void* attr);

		void buffer(void *ptr, char* buff);
		bool no_op(void* attr);
		bool eq_op(void* attr);
		bool ne_op(void* attr);
		bool lt_op(void* attr);
		bool gt_op(void* attr);
		bool le_op(void* attr);
		bool ge_op(void* attr);
		RC GiveNewPage(char *&data);
	};

	class RM_Manager {
		friend class QL_Manager;
	public:
		RM_Manager(PF_Manager &pfm);
		~RM_Manager();

		RC CreateFile(const char *fileName, int recordSize);
		RC DestroyFile(const char *fileName);
		RC OpenFile(const char *fileName, RM_FileHandle &fileHandle);

		RC CloseFile(RM_FileHandle &fileHandle);
	private:
		PF_Manager * pf_manager;
		int numRecordsPerPage(int recordSize);
	};

#endif // !RM_H
