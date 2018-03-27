#pragma once
#ifndef PF_INTERNAL_H
	#define PF_INTERNAL_H

	#pragma region Includes
		#include <cstdlib>
		#include <cstring>
		#include "pf.h"
	#pragma endregion

const int PF_BUFFER_SIZE = 40;
const int PF_HASH_TBL_SIZE = 20;

#define CREATION_MASK 0600 
	#define PF_PAGE_LIST_END -1    
#define PF_PAGE_USED -2

#ifndef L_SET
	#define L_SET 0
#endif

struct PF_PageHdr {
	int nextFree;
};

const int PF_FILE_HDR_SIZE = PF_PAGE_SIZE + sizeof(PF_PageHdr);

#endif // !PF_INTERNAL_H
