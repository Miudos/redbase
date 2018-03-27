#pragma region Includes
	#include <iostream>
	#include "pf_internal.h"
#pragma endregion

using namespace std;

static char *PF_WarnMsg[] = {
	(char*)"page pinned in buffer",
	(char*)"page is not in the buffer",
	(char*)"invalid page number",
	(char*)"file open",
	(char*)"invalid file descriptor (file closed)",
	(char*)"page already free",
	(char*)"page already unpinned",
	(char*)"end of file",
	(char*)"attempting to resize the buffer too small",
	(char*)"invalid filename"
};

static char *PF_ErrorMsg[] = {
	(char*)"no memory",
	(char*)"no buffer space",
	(char*)"incomplete read of page from file",
	(char*)"incomplete write of page to file",
	(char*)"incomplete read of header from file",
	(char*)"incomplete write of header from file",
	(char*)"new page to be allocated already in buffer",
	(char*)"hash table entry not found",
	(char*)"page already in hash table",
	(char*)"invalid file name"
};

void PF_PrintError(RC rc)
{
	if (rc >= START_PF_WARN && rc <= PF_LASTWARN)
		cerr << "PF warning: " << PF_WarnMsg[rc - START_PF_WARN] << "\n";
	else if (-rc >= -START_PF_ERR && -rc < -PF_LASTERROR)
		cerr << "PF error: " << PF_ErrorMsg[-rc + START_PF_ERR] << "\n";
	else if (rc == PF_UNIX)
			cerr << "OS error\n";
	else if (rc == 0)
		cerr << "PF_PrintError called with return code of 0\n";
	else
		cerr << "PF error: " << rc << " is out of bounds\n";
}