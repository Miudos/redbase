#pragma once
#ifndef SM_H
	#define SM_H

	#pragma region Includes
		#include <stdlib.h>
		#include <string.h>
		#include <vector>
		#include "redbase.h"
		#include "parser.h"
		#include "rm.h"
		#include "ix.h"
		#include "printer.h"
	#pragma endregion

	#pragma region Defines
		#define SM_ErrorForward(expr) do { \
			RC tmp_rc = (expr);\
			if (tmp_rc != OK_RC) \
				return ((tmp_rc > 0) ? WARN : ERR); \
		} while (0)

		#define SM_CREATE_WARN                      (START_SM_WARN + 0)
		#define SM_OPEN_WARN                        (START_SM_WARN + 1)
		#define SM_CLOSE_WARN                       (START_SM_WARN + 2)
		#define SM_BAD_INPUT                        (START_SM_WARN + 3)
		#define SM_DROP_WARN                        (START_SM_WARN + 4)
		#define SM_IXCREATE_WARN                    (START_SM_WARN + 5)
		#define SM_IXDROP_WARN                      (START_SM_WARN + 6)
		#define SM_DB_CLOSED                        (START_SM_WARN + 7)
		#define SM_LOAD_WARN                        (START_SM_WARN + 8)
		#define SM_DUPLICATE_RELATION               (START_SM_WARN + 9)
		#define SM_RELATION_NOT_FOUND               (START_SM_WARN + 10)
		#define SM_ATTRIBUTE_NOT_FOUND              (START_SM_WARN + 11)
		#define SM_PRINT_WARN                       (START_SM_WARN + 12)
		#define SM_NOT_IMPLEMENTED                  (START_SM_WARN + 13)
		#define SM_LASTWARN                         SM_NOT_IMPLEMENTED

		#define SM_CREATE_ERR                       (START_SM_ERR - 0)
		#define SM_OPEN_ERR                         (START_SM_ERR - 1)
		#define SM_CLOSE_ERR                        (START_SM_ERR - 2)
		#define SM_DROP_ERR                         (START_SM_ERR - 3)
		#define SM_IXCREATE_ERR                     (START_SM_ERR - 4)
		#define SM_IXDROP_ERR                       (START_SM_ERR - 5)
		#define SM_LOAD_ERR                         (START_SM_ERR - 6)
		#define SM_PRINT_ERR                        (START_SM_ERR - 7)
		#define SM_LASTERROR                        SM_PRINT_ERR
	#pragma endregion

	void SM_PrintError(RC rc);

	struct RelationInfo {
		RelationInfo() {
			memset((void*)this, 0, sizeof(*this));
		};

		char rel_name[MAXNAME + 1];
		int tuple_size;
		int num_attr;
		int index_num;
	};

	class SM_Manager {
		friend class QL_Manager;
	public:
		SM_Manager(IX_Manager &ixm, RM_Manager &rmm);
		~SM_Manager();

		RC OpenDb(const char *dbName);
		RC CloseDb();

		RC CreateTable(const char *relName, int attrCount, AttrInfo *attributes);
		RC CreateIndex(const char *relName, const char *attrName);
		RC DropTable(const char *relName);
		RC DropIndex(const char *relName, const char *attrName);
		RC Load(const char *relName, const char *fileName);
		RC Help();
		RC Help(const char *relName);

		RC Print(const char *relName);

		RC Set(const char *paramName,
			const char *value);
	private:
		IX_Manager *ixman;
		RM_Manager *rmman;
		bool isOpen;
		RM_FileHandle relcat;
		RM_FileHandle attrcat;
		RC getRelInfo(const char* relName, RM_Record &rec);
		RC getAttrInfo(const char* relName, const char* attrName, DataAttrInfo &dinfo, RM_Record &rec);
		RC getAttributes(const char *relName, std::vector<DataAttrInfo> &attributes);
		RC getRelation(const char* relName, RelationInfo &relation);

		bool SHOW_ALL_PLANS;
		int SORT_RES;
	};

	#define SM_ErrorForward(expr) do { \
	RC tmp_rc = (expr);\
	if (tmp_rc != OK_RC) \
		return ((tmp_rc > 0) ? WARN : ERR); \
	} while (0)

#endif // !SM_H
