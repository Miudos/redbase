#pragma once

#ifndef RM_RID_H
	#define RM_RID_H

	#pragma region Includes
		#include "redbase.h"
	#pragma endregion

	#pragma region Defines
		#ifndef RM_INVALID_RID
			#define RM_INVALID_RID (START_RM_WARN + 6)
		#endif // !RM_INVALID_RID

		#define RID_FLAG -1771 //TODO - RID_INITIAL_STATUS

		typedef int PageNum;
		typedef int SlotNum;
	#pragma endregion

	class RID {
		public:
			RID();
			RID(PageNum pageNum, SlotNum slotNum);
			~RID();
			RID(const RID &rid);
			RID& operator=(const RID &rid);
			bool operator==(const RID &rid) const;
			RC GetPageNum(PageNum &pageNum) const;
			RC GetSlotNum(SlotNum &slotNum) const;
		private:
			PageNum pageNum;
			SlotNum slotNum;
	};

#endif // !RM_RID_H
