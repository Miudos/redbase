#pragma once
#ifndef EX_H
	#define EX_H

	#pragma region Includes
		#include "pf.h"
		#include "ql.h"
	#pragma endregion

	#pragma region Defines
		#define EX_ErrorForward(expr) do { \
			RC tmp_rc = (expr);\
			if (tmp_rc != OK_RC) \
				return ((tmp_rc > 0) ? WARN : ERR); \
		} while (0)

		#define EX_MERGE_WARN 501
		#define EX_MERGE_ERR -501
	#pragma endregion

	void EX_PrintError(RC rc);

	struct EX_PageHdr {
		int numrecs;
	};

	class EX_Loader {
	public:
		EX_Loader(PF_Manager &pfm);
		~EX_Loader();
		RC Create(char *fileName, float ff, int recsize, bool makeIndex, int attrLength);
		RC PutRec(char* data);
		RC Close();
	private:
		PF_Manager * pfm;
		float ff;
		PF_FileHandle fh;
		bool isOpen;
		bool makeIndex;
		int attrLength;
		int recsize;
		int currPage;
		int recsInCurrPage;
		int capacity;
		EX_Loader* index;
	};

	class EX_Scanner {
	public:
		EX_Scanner(PF_Manager &pfm);
		~EX_Scanner();
		RC Open(const char* fileName, int startPage, bool goRight, int recsize);
		RC Next(std::vector<char> &rec);
		RC Reset();
		RC Close();
		RC NextBlock(char *block, int &numrecs);
	private:
		PF_Manager * pfm;
		PF_FileHandle fh;
		bool isOpen;
		int recsize;
		int currPage;
		int currSlot;
		int increment;
		int currPageForReset;
	};

	class EX_Sorter {
	public:
		EX_Sorter(PF_Manager &pfm, QL_Op &scan, int attrIndex);
		~EX_Sorter();
		RC sort(const char *fileName, float ff, bool makeIndex);
	private:
		PF_Manager * pfm;
		QL_Op *scan;
		int attrIndex;
		AttrType attrType;
		int offset;
		int attrLength;
		int recsize;
		int recsPerPage;
		int bufferSize;
		bool seenEOF;
		char *buffer;
		void pageSort(char* page, int lo, int hi);
		int partition(char* page, int lo, int hi);
		bool less(char *v, char *w);
		void exch(char *v, char *w);
		RC fillBuffer(std::vector<char*> &pages, std::vector<int> &numrecs);
		RC createSortedChunk(const char* fileName, int chunkNum, float ff, std::vector<char*> &pages, std::vector<int> &numrecs);
		int findIndex(std::vector<char*> &pages, std::vector<int> &numrecs, std::vector<int> &index);
		int pagesToReserve(QL_Op* node);
		void cleanUp(std::vector<char*> &pages, std::vector<int> &numrecs);
	};

	class BufferIterator {
	public:
		BufferIterator(PF_Manager *pfm, int recsize, int bufferSize);
		~BufferIterator();
		RC Clear();
		RC PutRec(std::vector<char> &rec);
		int Size();
		char* GetRec(int i);
	private:
		PF_Manager * pfm;
		int recsize;
		int bufferSize;
		int pagecap;
		int numrecs;
		int currPage;
		int currSlot;
		std::vector<char*> pages;
	};

	class EX_Sort : public QL_UnaryOp {
		friend class EX_Optimizer;
	public:
		EX_Sort(PF_Manager *pfm, QL_Op &child, int attrIndex);
		~EX_Sort();
		RC Open();
		RC Next(std::vector<char> &rec);
		RC Reset();
		RC Close();
	private:
		bool isOpen;
		bool isEmpty;
		int attrIndex;
		int recsize;
		bool deleteAtClose;
		char* fileName;
		PF_Manager* pfm;
		EX_Scanner* scanner;
		void gen_random(char* s, const int len);
	};

	class EX_MergeJoin : public QL_BinaryOp {
	public:
		EX_MergeJoin(PF_Manager* pfm, QL_Op &left, QL_Op &right, const Condition *cond);
		~EX_MergeJoin();
		RC Open();
		RC Next(std::vector<char> &rec);
		RC Reset();
		RC Close();
	private:
		bool isOpen;
		std::vector<char> leftrec;
		std::vector<char> rightrec;
		int leftRecSize;
		int rightRecSize;
		int bufferIndex;
		DataAttrInfo lattr;
		DataAttrInfo rattr;
		BufferIterator *buffit;
		bool rightEOF;
		bool less(char* left, char* right);
		bool more(char* left, char* right);
		bool equal(char* left, char* right);
	};

	class EX_Optimizer {
	public:
		EX_Optimizer(PF_Manager *pfm);
		~EX_Optimizer();
		void doSortMergeJoin(QL_Op* &root);
		void pushSort(QL_Op* &root);
		void doSortedScans(QL_Op* &root);
		void mergeProjections(QL_Op* &root);
	private:
		PF_Manager * pfm;
		bool okToSort(QL_Op* root);
	};
#endif // !EX_H
