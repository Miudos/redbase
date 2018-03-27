#pragma once
#ifndef PRINTER_H
	#define PRINTER_H

	#pragma region Includes
		#include <iostream>
		#include <cstring>
		#include "redbase.h"
	#pragma endregion

	#pragma region Defines
		#define MAXPRINTSTRING 53

		#ifndef minimum
		#define minimum(a,b) (((a) < (b)) ? (a) : (b))
		#endif

		#ifndef maximum
		#define maximum(a,b) (((a) > (b)) ? (a) : (b))
		#endif
	#pragma endregion

	void Spaces(int maxLength, int printedSoFar);

	struct DataAttrInfo
	{
		DataAttrInfo() {
			memset((void*)this, 0, sizeof(*this));
		};

		DataAttrInfo(const DataAttrInfo &d) {
			strcpy_s(relName, d.relName);
			strcpy_s(attrName, d.attrName);
			offset = d.offset;
			attrType = d.attrType;
			attrLength = d.attrLength;
			indexNo = d.indexNo;
		};

		DataAttrInfo& operator=(const DataAttrInfo &d) {
			if (this != &d) {
				strcpy_s(relName, d.relName);
				strcpy_s(attrName, d.attrName);
				offset = d.offset;
				attrType = d.attrType;
				attrLength = d.attrLength;
				indexNo = d.indexNo;
			}
			return (*this);
		};

		bool operator<(const DataAttrInfo &d) const {
			return (this->offset < d.offset);
		};

		char relName[MAXNAME + 1];
		char attrName[MAXNAME + 1];
		int offset;
		AttrType attrType;
		int attrLength;
		int indexNo;
	};

	class Printer {
	public:
		Printer(const DataAttrInfo *attributes, const int attrCount);
		~Printer();

		void PrintHeader(std::ostream &c) const;
		void Print(std::ostream &c, const char * const data);
		void Print(std::ostream &c, const void * const data[]);

		void PrintFooter(std::ostream &c) const;

	private:
		DataAttrInfo * attributes;
		int attrCount;

		char **psHeader;
		int *spaces;
		int iCount;
	};

#endif // !PRINTER_H
