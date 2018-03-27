#pragma region Includes
	#include <iostream>

	#include "ql.h"
#pragma endregion

using namespace std;


static char *QL_WarnMsg[] = {
	(char*)"recoverable error during insert",
	(char*)"attempt to modify catalog",
	(char*)"invalid input parameters encountered",
	(char*)"recoverable error during delete",
	(char*)"recoverable error during file scan",
	(char*)"end of file reached",
	(char*)"recoverable error during index scan",
	(char*)"recoverable error during update",
	(char*)"recoverable error in the condition operator",
	(char*)"recoverable error during select",
	(char*)"recoverable error in the cross operator",
	(char*)"recoverable error in the projection operator",
	(char*)"recoverable error in the permute/duplicate operator"
};

static char *QL_ErrorMsg[] = {
	(char*)"fatal error during insert",
	(char*)"fatal error during delete",
	(char*)"fatal error during file scan",
	(char*)"fatal error during index scan",
	(char*)"fatal error during update",
	(char*)"fatal error in the condition operator",
	(char*)"fatal error during select",
	(char*)"fatal error in the cross product operator",
	(char*)"fatal error in the projection operator",
	(char*)"fatal error in the permute/duplicate operator"
};

void QL_PrintError(RC rc)
{
	if (rc >= START_QL_WARN && rc <= QL_LASTWARN)
		cerr << "QL warning: " << QL_WarnMsg[rc - START_QL_WARN] << "\n";
	else if (-rc >= -START_QL_ERR && -rc <= -QL_LASTERROR)
		cerr << "QL error: " << QL_ErrorMsg[-rc + START_QL_ERR] << "\n";
	else if (rc == 0)
		cerr << "QL_PrintError called with return code of 0\n";
	else
		cerr << "QL error: " << rc << " is out of bounds\n";
}