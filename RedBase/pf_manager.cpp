#pragma region Includes
	#include <fcntl.h>
	#include <sys\types.h>
	#include <sys\stat.h>
	#include <share.h>
	#include <io.h> //Required to avoid errors in write attributes and relations

	#include <fstream>
	#include <cstring> // Required to compile successfully this code. VS 2017 uses c11 and c++11 which does not has strerrorlen_s, essencial to replace strerror with strerror_s
	#include <corecrt_io.h>

	#include "pf_internal.h"
	#include "pf_buffermgr.h"
#pragma endregion

PF_Manager::PF_Manager()
{
	pBufferMgr = new PF_BufferMgr(PF_BUFFER_SIZE);
}

PF_Manager::~PF_Manager()
{
	delete pBufferMgr;
}

RC PF_Manager::CreateFile(const char *fileName)
{
	int fh = 0;
	int numBytes;

	errno_t err = _sopen_s(&fh, fileName, _O_BINARY | _O_RDWR | _O_CREAT, _SH_DENYNO, _S_IREAD | _S_IWRITE);

	if (err !=0) {
		printf_s("%d %d\n", err, fh);
		//printf(strerror(errno));
		_unlink(fileName);
		return (PF_UNIX);
	}
	char hdrBuf[PF_FILE_HDR_SIZE];
	memset(hdrBuf, 0, PF_FILE_HDR_SIZE);

	PF_FileHdr *hdr = (PF_FileHdr*)hdrBuf;
	hdr->firstFree = PF_PAGE_LIST_END;
	hdr->numPages = 0;

	numBytes = _write(fh, hdrBuf, PF_FILE_HDR_SIZE);

	if (numBytes == -1) {
		printf_s("%d %d\n", numBytes, fh);
		//printf(strerror(errno));
		_close(fh);
		_unlink(fileName);
		if (numBytes < 0)
			return (PF_UNIX);
		else
			return (PF_HDRWRITE);
	}

	if (_close(fh) < 0)
		return (PF_UNIX);
	return (0);
}

RC PF_Manager::DestroyFile(const char *fileName)
{
	if (_unlink(fileName) < 0)
		return (PF_UNIX);
	return (0);
}

RC PF_Manager::OpenFile(const char *fileName, PF_FileHandle &fileHandle)
{
	int rc;
	if (fileHandle.bFileOpen)
		return (PF_FILEOPEN);

	errno_t err = _sopen_s(&fileHandle.fh, fileName, _O_BINARY | _O_RDWR | _O_CREAT, _SH_DENYNO, _S_IREAD | _S_IWRITE);

	if (err != 0) {
		printf_s("%d %d\n", err, &fileHandle.fh);
		return (PF_UNIX);
	}

	{
		int numBytes = _read(fileHandle.fh, (char *)&fileHandle.hdr, sizeof(PF_FileHdr));
		if (numBytes != sizeof(PF_FileHdr)) {
			rc = (numBytes < 0) ? PF_UNIX : PF_HDRREAD;
			goto err;
		}
	}

	fileHandle.bHdrChanged = FALSE;

	fileHandle.pBufferMgr = pBufferMgr;
	fileHandle.bFileOpen = TRUE;
	return 0;

err:
	_close(fileHandle.fh);
	fileHandle.bFileOpen = FALSE;
	return (rc);
}

RC PF_Manager::CloseFile(PF_FileHandle &fileHandle)
{
	RC rc;

	if (!fileHandle.bFileOpen)
		return (PF_CLOSEDFILE);

	if ((rc = fileHandle.FlushPages()))
		return (rc);
	if (_close(fileHandle.fh) < 0)
		return (PF_UNIX);
	fileHandle.bFileOpen = FALSE;
	fileHandle.pBufferMgr = NULL;
	return 0;
}

RC PF_Manager::ClearBuffer()
{
	return pBufferMgr->ClearBuffer();
}

RC PF_Manager::PrintBuffer()
{
	return pBufferMgr->PrintBuffer();
}

RC PF_Manager::ResizeBuffer(int iNewSize)
{
	return pBufferMgr->ResizeBuffer(iNewSize);
}

RC PF_Manager::GetBlockSize(int &length) const
{
	return pBufferMgr->GetBlockSize(length);
}

RC PF_Manager::AllocateBlock(char *&buffer)
{
	return pBufferMgr->AllocateBlock(buffer);
}

RC PF_Manager::DisposeBlock(char *buffer)
{
	return pBufferMgr->DisposeBlock(buffer);
}
