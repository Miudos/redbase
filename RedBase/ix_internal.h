#pragma once
#ifndef IX_INTERNAL_H
	#define IX_INTERNAL_H

	#pragma region Defines
		#define IX_SENTINEL -1

		#define IX_ErrorForward(expr) do { \
			RC tmp_rc = (expr);\
			if (tmp_rc != OK_RC) \
					return ((tmp_rc > 0) ? WARN : ERR); \
			} while (0)
	#pragma endregion

	enum IPageType {
		INTERNAL,
		LEAF
	};

	struct IX_InternalHdr {
		IPageType type;
		int num_keys;
		int left_pnum;
	};

	struct IX_LeafHdr {
		IPageType type;
		int num_keys;
		int left_pnum;
		int right_pnum;
	};

	struct IX_OverflowHdr {
		int num_rids;
		int next_page;
	};

#endif // !IX_INTERNAL_H
