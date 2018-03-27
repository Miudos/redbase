#pragma region Includes
	#include "rm.h"
#pragma endregion

using namespace std;

RM_Record::RM_Record() {
	bIsAllocated = 0;
}

RM_Record::~RM_Record() {
	if (bIsAllocated) delete[] record;
}

RC RM_Record::GetData(char *&pData) const {
	if (!bIsAllocated) return RM_INVALID_RECORD;
	pData = record;
	return OK_RC;
}

RC RM_Record::GetRid(RID &rid) const {
	if (!bIsAllocated) return RM_INVALID_RECORD;
	rid = this->rid;
	return OK_RC;
}