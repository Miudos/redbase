#pragma once

#ifndef RM_INTERNAL_H
	#define RM_INTERNAL_H

struct RM_PageHdr {
	int next_free;
	int num_recs;
};

#define RM_ErrorForward(expr) do { \
RC tmp_rc = (expr);\
if (tmp_rc != OK_RC) \
        return ((tmp_rc > 0) ? WARN : ERR); \
} while (0)

#define RM_SENTINEL -1

#endif // !RM_INTERNAL_H
