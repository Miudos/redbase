#pragma once
#ifndef QL_H
#define QL_H

	#pragma region Includes
		#include "redbase.h"
		#include "parser.h"
		#include "rm.h"
		#include "ix.h"
		#include "sm.h"
	#pragma endregion

	void QL_PrintError(RC rc);

	class QL_Manager {
		friend class QL_Condition;
		friend class QL_Optimizer;
		friend class QL_Projection;
		friend class EX_Sorter;
		friend class EX_MergeJoin;
		friend class EX_Optimizer;
	public:
		QL_Manager(SM_Manager &smm, IX_Manager &ixm, RM_Manager &rmm);
		~QL_Manager();

		RC Select(int nSelAttrs, const RelAttr selAttrs[], int   nRelations, const char * const relations[], int nConditions, const Condition conditions[]);
		RC Insert(const char *relName, int nValues, const Value values[]);
		RC Delete(const char *relName, int nConditions, const Condition conditions[]);
		RC Update(const char *relName, const RelAttr &updAttr, const int bIsValue, const RelAttr &rhsRelAttr, const Value &rhsValue, int nConditions, const Condition conditions[]);
	private:
		RM_Manager *rmm;
		IX_Manager *ixm;
		SM_Manager *smm;
		static bool isValidAttr(char* attrName, const std::vector<DataAttrInfo> &attributes);
		static int indexToUse(int nConditions, const Condition conditions[],const std::vector<DataAttrInfo> &attributes);
		static void buffer(void* ptr, char* buff, int len);
		static bool eq_op(void* attr1, void* attr2, int len1, int len2, AttrType type);
		static bool ne_op(void* attr1, void* attr2, int len1, int len2, AttrType type);
		static bool lt_op(void* attr1, void* attr2, int len1, int len2, AttrType type);
		static bool gt_op(void* attr1, void* attr2, int len1, int len2, AttrType type);
		static bool ge_op(void* attr1, void* attr2, int len1, int len2, AttrType type);
		static bool le_op(void* attr1, void* attr2, int len1, int len2, AttrType type);
		static bool evalCondition(void* data, const Condition &cond, const std::vector<DataAttrInfo> &attributes);
		static bool isValidCondition(const Condition &cond, const std::vector<DataAttrInfo> &attributes);
		static int findAttr(const char* relName, const char *attrName, const std::vector<DataAttrInfo> &attributes);
		void printPlanHeader(const char *operation, const char* relname);
		void printPlanFooter();
	};

	#pragma region Defines
		#define QL_ErrorForward(expr) do { \
				RC tmp_rc = (expr);\
				if (tmp_rc != OK_RC) \
					return ((tmp_rc > 0) ? WARN : ERR); \
				} while (0)

		#define QL_INSERT_WARN                (START_QL_WARN + 0)
		#define QL_CAT_WARN                   (START_QL_WARN + 1)
		#define QL_INVALID_WARN               (START_QL_WARN + 2)
		#define QL_DELETE_WARN                (START_QL_WARN + 3)
		#define QL_FILESCAN_WARN              (START_QL_WARN + 4)
		#define QL_EOF                        (START_QL_WARN + 5)
		#define QL_IXSCAN_WARN                (START_QL_WARN + 6)
		#define QL_UPDATE_WARN                (START_QL_WARN + 7)
		#define QL_COND_WARN                  (START_QL_WARN + 8)
		#define QL_SELECT_WARN                (START_QL_WARN + 9)
		#define QL_CROSS_WARN                 (START_QL_WARN + 10)
		#define QL_PROJ_WARN                  (START_QL_WARN + 11)
		#define QL_PERMDUP_WARN               (START_QL_WARN + 12)
		#define QL_LASTWARN                    QL_PERMDUP_WARN

		#define QL_INSERT_ERR                (START_QL_ERR - 0)
		#define QL_DELETE_ERR                (START_QL_ERR - 1)
		#define QL_FILESCAN_ERR              (START_QL_ERR - 2)
		#define QL_IXSCAN_ERR                (START_QL_ERR - 3)
		#define QL_UPDATE_ERR                (START_QL_ERR - 4)
		#define QL_COND_ERR                  (START_QL_ERR - 5)
		#define QL_SELECT_ERR                (START_QL_ERR - 6)
		#define QL_CROSS_ERR                 (START_QL_ERR - 7)
		#define QL_PROJ_ERR                  (START_QL_ERR - 8)
		#define QL_PERMDUP_ERR               (START_QL_ERR - 9)
		#define QL_LASTERROR                  QL_PERMDUP_ERR
	#pragma endregion

#endif // !QL_H
