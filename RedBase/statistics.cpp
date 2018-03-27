#pragma region Includes
	#include <string.h> //required to compile successfully linkedlist.h
	#include <stdio.h>

	#include <iostream>
	#include "statistics.h"
#pragma endregion

using namespace std;

const char *PF_GETPAGE = "GETPAGE";
const char *PF_PAGEFOUND = "PAGEFOUND";
const char *PF_PAGENOTFOUND = "PAGENOTFOUND";
const char *PF_READPAGE = "READPAGE";
const char *PF_WRITEPAGE = "WRITEPAGE";
const char *PF_FLUSHPAGES = "FLUSHPAGES";

Statistic::Statistic()
{
	psKey = NULL;
	iValue = 0;
}

Statistic::Statistic(const char *psKey_)
{
	psKey = new char[strlen(psKey_) + 1];
	strcpy_s(psKey, sizeof psKey, psKey_);

	iValue = 0;
}

Statistic::Statistic(const Statistic &stat)
{
	psKey = new char[strlen(stat.psKey) + 1];
	strcpy_s(psKey, sizeof psKey, stat.psKey);

	iValue = stat.iValue;
}

Statistic& Statistic::operator=(const Statistic &stat)
{
	if (this == &stat)
		return *this;

	delete[] psKey;
	psKey = new char[strlen(stat.psKey) + 1];
	strcpy_s(psKey, sizeof psKey, stat.psKey);

	iValue = stat.iValue;

	return *this;
}

Statistic::~Statistic()
{
	delete[] psKey;
}

Boolean Statistic::operator==(const char *psKey_) const
{
	return (strcmp(psKey_, psKey) == 0);
}

RC StatisticsMgr::Register(const char *psKey, const Stat_Operation op, const int *const piValue)
{
	int i, iCount;
	Statistic *pStat = NULL;

	if (psKey == NULL || (op != STAT_ADDONE && piValue == NULL))
		return STAT_INVALID_ARGS;

	iCount = llStats.GetLength();

	for (i = 0; i < iCount; i++) {
		pStat = llStats[i];
		if (*pStat == psKey)
			break;
	}

	if (i == iCount)
		pStat = new Statistic(psKey);

	switch (op) {
	case STAT_ADDONE:
		pStat->iValue++;
		break;
	case STAT_ADDVALUE:
		pStat->iValue += *piValue;
		break;
	case STAT_SETVALUE:
		pStat->iValue = *piValue;
		break;
	case STAT_MULTVALUE:
		pStat->iValue *= *piValue;
		break;
	case STAT_DIVVALUE:
		pStat->iValue = (int)(pStat->iValue / (*piValue));
		break;
	case STAT_SUBVALUE:
		pStat->iValue -= *piValue;
		break;
	};

	if (i == iCount) {
		llStats.Append(*pStat);
		delete pStat;
	}

	return 0;
}

RC StatisticsMgr::Print(const char *psKey)
{
	if (psKey == NULL)
		return STAT_INVALID_ARGS;

	int *iValue = Get(psKey);

	if (iValue)
		cout << psKey << "::" << *iValue << "\n";
	else
		return STAT_UNKNOWN_KEY;

	delete iValue;

	return 0;
}

int *StatisticsMgr::Get(const char *psKey)
{
	int i, iCount;
	Statistic *pStat = NULL;

	iCount = llStats.GetLength();

	for (i = 0; i < iCount; i++) {
		pStat = llStats[i];
		if (*pStat == psKey)
			break;
	}
	if (i == iCount)
		return NULL;

	return new int(pStat->iValue);
}

void StatisticsMgr::Print()
{
	int i, iCount;
	Statistic *pStat = NULL;

	iCount = llStats.GetLength();

	for (i = 0; i < iCount; i++) {
		pStat = llStats[i];
		cout << pStat->psKey << "::" << pStat->iValue << "\n";
	}
}

RC StatisticsMgr::Reset(const char *psKey)
{
	int i, iCount;
	Statistic *pStat = NULL;

	if (psKey == NULL)
		return STAT_INVALID_ARGS;

	iCount = llStats.GetLength();

	for (i = 0; i < iCount; i++) {
		pStat = llStats[i];
		if (*pStat == psKey)
			break;
	}

	if (i != iCount)
		llStats.Delete(i);
	else
		return STAT_UNKNOWN_KEY;

	return 0;
}

void StatisticsMgr::Reset()
{
	llStats.Erase();
}
