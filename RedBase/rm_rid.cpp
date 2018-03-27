#pragma region Includes
	#include <cstdio>
	#include <iostream>
	#include "rm_rid.h"
#pragma endregion

using namespace std;

RID::RID() {
	pageNum = RID_FLAG;
	slotNum = RID_FLAG;
}

RID::RID(PageNum pageNum, SlotNum slotNum) {
	this->pageNum = pageNum;
	this->slotNum = slotNum;
}

RID::~RID() {}

RID::RID(const RID &rid) {
	this->pageNum = rid.pageNum;
	this->slotNum = rid.slotNum;
}

RID& RID::operator=(const RID &rid) {
	if (this != &rid) {
		this->pageNum = rid.pageNum;
		this->slotNum = rid.slotNum;
	}
	return (*this);
}

bool RID::operator==(const RID &rid) const {
	return (this->pageNum == rid.pageNum) && (this->slotNum == rid.slotNum);
}

RC RID::GetPageNum(PageNum &pageNum) const {
	if (this->pageNum == RID_FLAG) return RM_INVALID_RID;
	pageNum = this->pageNum;
	return OK_RC;
}

RC RID::GetSlotNum(SlotNum &slotNum) const {
	if (this->slotNum == RID_FLAG) return RM_INVALID_RID;
	slotNum = this->slotNum;
	return OK_RC;
}