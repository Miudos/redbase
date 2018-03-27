#pragma region Includes
	#include <iostream>
	#include "sm.h"
#pragma endregion

using namespace std;

static char *SM_WarnMsg[] = {
	(char*)"recoverable error during creation of relation",
	(char*)"recoverable error during opening db",
	(char*)"recoverable error during closing db",
	(char*)"invalid input parameters encountered",
	(char*)"recoverable error while dropping a relation",
	(char*)"recoverable error while creating index",
	(char*)"recoverable error while dropping index",
	(char*)"attempt to acccess a closed db",
	(char*)"recoverable error while loading into a relation",
	(char*)"relation with the given name already exists",
	(char*)"the given relation doesn't exist",
	(char*)"the relation doesn't have the given attribute",
	(char*)"recoverable error while printing relation",
	(char*)"the given function hasn't been implemented"
};

static char *SM_ErrorMsg[] = {
	(char*)"fatal error during db creation",
	(char*)"fatal error while opening db",
	(char*)"fatal error while closing db",
	(char*)"fatal error while dropping relation",
	(char*)"fatal error while creating index",
	(char*)"fatal error while dropping index",
	(char*)"fatal error while loading into a relation",
	(char*)"fatal error while printing relation"
};

void SM_PrintError(RC rc)
{
	if (rc >= START_SM_WARN && rc <= SM_LASTWARN)
		cerr << "SM warning: " << SM_WarnMsg[rc - START_SM_WARN] << "\n";
	else if (-rc >= -START_SM_ERR && -rc <= -SM_LASTERROR)
		cerr << "SM error: " << SM_ErrorMsg[-rc + START_SM_ERR] << "\n";
	else if (rc == 0)
		cerr << "SM_PrintError called with return code of 0\n";
	else
		cerr << "SM error: " << rc << " is out of bounds\n";
}