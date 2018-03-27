#pragma region Includes

	#include "printer.h"
#pragma endregion

using namespace std;

void Spaces(int maxLength, int printedSoFar)
{
	for (int i = printedSoFar; i < maxLength; i++)
		cout << " ";
}

Printer::Printer(const DataAttrInfo *attributes_, const int attrCount_)
{
	attrCount = attrCount_;
	attributes = new DataAttrInfo[attrCount];

	for (int i = 0; i < attrCount; i++)
		attributes[i] = attributes_[i];

	iCount = 0;
	psHeader = (char**)malloc(attrCount * sizeof(char*));

	spaces = new int[attrCount];

	for (int i = 0; i < attrCount; i++) {
		int bFound = 0;
		psHeader[i] = new char[MAXPRINTSTRING];
		memset(psHeader[i], 0, MAXPRINTSTRING);

		for (int j = 0; j < attrCount; j++)
			if (j != i &&
				strcmp(attributes[i].attrName,
					attributes[j].attrName) == 0) {
				bFound = 1;
				break;
			}

		if (bFound)
			printf_s(psHeader[i], "%s.%s",
				attributes[i].relName, attributes[i].attrName);
		else
			strcpy_s(psHeader[i],strlen(attributes[i].attrName)+1, attributes[i].attrName);

		if (attributes[i].attrType == STRING)
			spaces[i] = minimum(attributes[i].attrLength, MAXPRINTSTRING);
		else
			spaces[i] = maximum(12, strlen(psHeader[i]));

		spaces[i] -= strlen(psHeader[i]);

		if (spaces[i] < 1) {
			spaces[i] = 0;
			strcat_s(psHeader[i], 2, " ");
		}
	}
}

Printer::~Printer()
{
	for (int i = 0; i < attrCount; i++)
		delete[] psHeader[i];

	delete[] spaces;
	free(psHeader);
	delete[] attributes;
}

void Printer::PrintHeader(ostream &c) const
{
	int dashes = 0;
	int iLen;
	int i, j;

	for (i = 0; i < attrCount; i++) {
		c << psHeader[i];
		iLen = strlen(psHeader[i]);
		dashes += iLen;

		for (j = 0; j < spaces[i]; j++)
			c << " ";

		dashes += spaces[i];
	}

	c << "\n";
	for (i = 0; i < dashes; i++) c << "-";
	c << "\n";
}

void Printer::PrintFooter(ostream &c) const
{
	c << "\n";
	c << iCount << " tuple(s).\n";
}

void Printer::Print(ostream &c, const void * const data[])
{
	char* str = new char[MAXPRINTSTRING];
	char strSpace[50];
	int i, a;
	float b;

	iCount++;

	for (i = 0; i<attrCount; i++) {
		if (attributes[i].attrType == STRING) {
			memset(str, 0, MAXPRINTSTRING);

			if (attributes[i].attrLength>MAXPRINTSTRING) {
				strncpy_s(str, strlen((char*)data[i]) + 1, (char *)data[i], MAXPRINTSTRING - 1);
				str[MAXPRINTSTRING - 3] = '.';
				str[MAXPRINTSTRING - 2] = '.';
				c << str;
				Spaces(MAXPRINTSTRING, strlen(str));
			}
			else {
				strncpy_s(str, strlen((char*)data[i])+1, (char *)data[i], attributes[i].attrLength);
				c << str;
				if (attributes[i].attrLength < (int)strlen(psHeader[i]))
					Spaces(strlen(psHeader[i]), strlen(str));
				else
					Spaces(attributes[i].attrLength, strlen(str));
			}
		}
		if (attributes[i].attrType == INT) {
			memcpy(&a, data[i], sizeof(int));
			sprintf_s(strSpace, "%d", a);
			c << a;
			if (strlen(psHeader[i]) < 12)
				Spaces(12, strlen(strSpace));
			else
				Spaces(strlen(psHeader[i]), strlen(strSpace));
		}
		if (attributes[i].attrType == FLOAT) {
			memcpy(&b, data[i], sizeof(float));
			sprintf_s(strSpace, "%f", b);
			c << strSpace;
			if (strlen(psHeader[i]) < 12)
				Spaces(12, strlen(strSpace));
			else
				Spaces(strlen(psHeader[i]), strlen(strSpace));
		}
	}
	c << "\n";
}

void Printer::Print(ostream &c, const char * const data)
{
	char* str = new char[MAXPRINTSTRING];
	char strSpace[50];
	int i, a;
	float b;

	if (data == NULL)
		return;
	iCount++;

	for (i = 0; i<attrCount; i++) {
		if (attributes[i].attrType == STRING) {
			memset(str, 0, MAXPRINTSTRING);

			if (attributes[i].attrLength>MAXPRINTSTRING) {
				strncpy_s(str, strlen(data + attributes[i].offset)+1, data + attributes[i].offset, MAXPRINTSTRING - 1);
				str[MAXPRINTSTRING - 3] = '.';
				str[MAXPRINTSTRING - 2] = '.';
				c << str;
				Spaces(MAXPRINTSTRING, strlen(str));
			}
			else {
				strncpy_s(str,strlen(data + attributes[i].offset)+1, data + attributes[i].offset, attributes[i].attrLength);
				c << str;
				if (attributes[i].attrLength < (int)strlen(psHeader[i]))
					Spaces(strlen(psHeader[i]), strlen(str));
				else
					Spaces(attributes[i].attrLength, strlen(str));
			}
		}
		if (attributes[i].attrType == INT) {
			memcpy(&a, (data + attributes[i].offset), sizeof(int));
			sprintf_s(strSpace, "%d", a);
			c << a;
			if (strlen(psHeader[i]) < 12)
				Spaces(12, strlen(strSpace));
			else
				Spaces(strlen(psHeader[i]), strlen(strSpace));
		}
		if (attributes[i].attrType == FLOAT) {
			memcpy(&b, (data + attributes[i].offset), sizeof(float));
			sprintf_s(strSpace, "%f", b);
			c << strSpace;
			if (strlen(psHeader[i]) < 12)
				Spaces(12, strlen(strSpace));
			else
				Spaces(strlen(psHeader[i]), strlen(strSpace));
		}
	}
	c << "\n";
}