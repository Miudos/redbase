#pragma once
#ifndef STATISTICS_H
	#define STATISTICS_H

	#pragma region Includes
		#include "linkedlist.h"
	#pragma endregion

	class Statistic {
	public:
		Statistic();
		~Statistic();
		Statistic(const char *psName);
		Statistic(const Statistic &stat);
		Statistic& operator=(const Statistic &stat);

		Boolean operator==(const char *psName_) const;
		char *psKey;
		int iValue;
	};

	enum Stat_Operation {
		STAT_ADDONE,
		STAT_ADDVALUE,
		STAT_SETVALUE,
		STAT_MULTVALUE,
		STAT_DIVVALUE,
		STAT_SUBVALUE
	};

	class StatisticsMgr {

	public:
		StatisticsMgr() {};
		~StatisticsMgr() {};

		RC Register(const char *psKey, const Stat_Operation op, const int *const piValue = NULL);
		int *Get(const char *psKey);

		RC Print(const char *psKey);
		void Print();
		RC Reset(const char *psKey);
		void Reset();

	private:
		LinkList<Statistic> llStats;
	};

	const int STAT_INVALID_ARGS = STAT_BASE + 1;
	const int STAT_UNKNOWN_KEY = STAT_BASE + 2;
	extern const char *PF_GETPAGE;
	extern const char *PF_PAGEFOUND;
	extern const char *PF_PAGENOTFOUND;
	extern const char *PF_READPAGE;
	extern const char *PF_WRITEPAGE;
	extern const char *PF_FLUSHPAGES;
#endif // !STATISTICS_H
