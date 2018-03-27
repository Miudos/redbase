#ifdef STATISTICS_H
	#pragma region Includes
		#include <iostream>
		#include "pf.h"
		#include "statistics.h"
	#pragma endregion

	using namespace std;

	extern StatisticsMgr *pStatisticsMgr;

	void PF_Statistics()
	{
		int *piGP = pStatisticsMgr->Get(PF_GETPAGE);
		int *piPF = pStatisticsMgr->Get(PF_PAGEFOUND);
		int *piPNF = pStatisticsMgr->Get(PF_PAGENOTFOUND);
		int *piRP = pStatisticsMgr->Get(PF_READPAGE);
		int *piWP = pStatisticsMgr->Get(PF_WRITEPAGE);
		int *piFP = pStatisticsMgr->Get(PF_FLUSHPAGES);

		cout << "PF Layer Statistics\n";
		cout << "-------------------\n";

		cout << "Total number of calls to GetPage Routine: ";
		if (piGP) cout << *piGP; else cout << "None";
		cout << "\n  Number found: ";
		if (piPF) cout << *piPF; else cout << "None";
		cout << "\n  Number not found: ";
		if (piPNF) cout << *piPNF; else cout << "None";
		cout << "\n-------------------\n";

		cout << "Number of read requests: ";
		if (piRP) cout << *piRP; else cout << "None";
		cout << "\nNumber of write requests: ";
		if (piWP) cout << *piWP; else cout << "None";
		cout << "\n-------------------\n";
		cout << "Number of flushes: ";
		if (piFP) cout << *piFP; else cout << "None";
		cout << "\n-------------------\n";

		delete piGP;
		delete piPF;
		delete piPNF;
		delete piRP;
		delete piWP;
		delete piFP;
	}
#endif // STATISTICS_H
