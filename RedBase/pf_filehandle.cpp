#pragma region Includes
	#include <io.h>

	#include "pf_buffermgr.h"
#pragma endregion

PF_FileHandle::PF_FileHandle()
{
	bFileOpen = FALSE;
	pBufferMgr = NULL;
}

PF_FileHandle::~PF_FileHandle(){}

PF_FileHandle::PF_FileHandle(const PF_FileHandle &fileHandle)
{
	this->pBufferMgr = fileHandle.pBufferMgr;
	this->hdr = fileHandle.hdr;
	this->bFileOpen = fileHandle.bFileOpen;
	this->bHdrChanged = fileHandle.bHdrChanged;
	this->fh = fileHandle.fh;
}

PF_FileHandle& PF_FileHandle::operator= (const PF_FileHandle &fileHandle)
{
	if (this != &fileHandle) {

		this->pBufferMgr = fileHandle.pBufferMgr;
		this->hdr = fileHandle.hdr;
		this->bFileOpen = fileHandle.bFileOpen;
		this->bHdrChanged = fileHandle.bHdrChanged;
		this->fh = fileHandle.fh;
	}

	return (*this);
}

RC PF_FileHandle::GetFirstPage(PF_PageHandle &pageHandle) const
{
	return (GetNextPage((PageNum)-1, pageHandle));
}

RC PF_FileHandle::GetLastPage(PF_PageHandle &pageHandle) const
{
	return (GetPrevPage((PageNum)hdr.numPages, pageHandle));
}

RC PF_FileHandle::GetNextPage(PageNum current, PF_PageHandle &pageHandle) const
{
	int rc;
	if (!bFileOpen)
		return (PF_CLOSEDFILE);

	if (current != -1 && !IsValidPageNum(current))
		return (PF_INVALIDPAGE);

	for (current++; current < hdr.numPages; current++) {
		if (!(rc = GetThisPage(current, pageHandle)))
			return (0);

		if (rc != PF_INVALIDPAGE)
			return (rc);
	}
	return (PF_EOF);
}

RC PF_FileHandle::GetPrevPage(PageNum current, PF_PageHandle &pageHandle) const
{
	int rc;
	if (!bFileOpen)
		return (PF_CLOSEDFILE);

	if (current != hdr.numPages && !IsValidPageNum(current))
		return (PF_INVALIDPAGE);
	for (current--; current >= 0; current--) {
		if (!(rc = GetThisPage(current, pageHandle)))
			return (0);

		if (rc != PF_INVALIDPAGE)
			return (rc);
	}
	return (PF_EOF);
}

RC PF_FileHandle::GetThisPage(PageNum pageNum, PF_PageHandle &pageHandle) const
{
	int  rc;
	char *pPageBuf;
	if (!bFileOpen)
		return (PF_CLOSEDFILE);

	if (!IsValidPageNum(pageNum))
		return (PF_INVALIDPAGE);
	if ((rc = pBufferMgr->GetPage(fh, pageNum, &pPageBuf)))
		return (rc);
	if (((PF_PageHdr*)pPageBuf)->nextFree == PF_PAGE_USED) {
		pageHandle.pageNum = pageNum;
		pageHandle.pPageData = pPageBuf + sizeof(PF_PageHdr);

		return (0);
	}
	if ((rc = UnpinPage(pageNum)))
		return (rc);
	return (PF_INVALIDPAGE);
}

RC PF_FileHandle::AllocatePage(PF_PageHandle &pageHandle)
{
	int rc;
	int pageNum;
	char *pPageBuf;
	if (!bFileOpen)
		return (PF_CLOSEDFILE);

	if (hdr.firstFree != PF_PAGE_LIST_END) {
		pageNum = hdr.firstFree;
		if ((rc = pBufferMgr->GetPage(fh,
			pageNum,
			&pPageBuf)))
			return (rc);
		hdr.firstFree = ((PF_PageHdr*)pPageBuf)->nextFree;
	}
	else {
		pageNum = hdr.numPages;
		if ((rc = pBufferMgr->AllocatePage(fh,
			pageNum,
			&pPageBuf)))
			return (rc);
		hdr.numPages++;
	}

	bHdrChanged = TRUE;
	((PF_PageHdr *)pPageBuf)->nextFree = PF_PAGE_USED;
	memset(pPageBuf + sizeof(PF_PageHdr), 0, PF_PAGE_SIZE);
	if ((rc = MarkDirty(pageNum)))
		return (rc);
	pageHandle.pageNum = pageNum;
	pageHandle.pPageData = pPageBuf + sizeof(PF_PageHdr);
	return (0);
}

RC PF_FileHandle::DisposePage(PageNum pageNum)
{
	int rc;
	char *pPageBuf;
	if (!bFileOpen)
		return (PF_CLOSEDFILE);
	if (!IsValidPageNum(pageNum))
		return (PF_INVALIDPAGE);

	if ((rc = pBufferMgr->GetPage(fh,
		pageNum,
		&pPageBuf,
		FALSE)))
		return (rc);
	if (((PF_PageHdr *)pPageBuf)->nextFree != PF_PAGE_USED) {
		if ((rc = UnpinPage(pageNum)))
			return (rc);
		return (PF_PAGEFREE);
	}

	((PF_PageHdr *)pPageBuf)->nextFree = hdr.firstFree;
	hdr.firstFree = pageNum;
	bHdrChanged = TRUE;

	if ((rc = MarkDirty(pageNum)))
		return (rc);

	if ((rc = UnpinPage(pageNum)))
		return (rc);
	return (0);
}

RC PF_FileHandle::MarkDirty(PageNum pageNum) const
{
	if (!bFileOpen)
		return (PF_CLOSEDFILE);
	if (!IsValidPageNum(pageNum))
		return (PF_INVALIDPAGE);

	return (pBufferMgr->MarkDirty(fh, pageNum));
}

RC PF_FileHandle::UnpinPage(PageNum pageNum) const
{
	if (!bFileOpen)
		return (PF_CLOSEDFILE);
	if (!IsValidPageNum(pageNum))
		return (PF_INVALIDPAGE);
	return (pBufferMgr->UnpinPage(fh, pageNum));
}

RC PF_FileHandle::FlushPages() const
{
	if (!bFileOpen)
		return (PF_CLOSEDFILE);
	if (bHdrChanged) {
		if (_lseek(fh, 0, L_SET) < 0)
			return (PF_UNIX);
		int numBytes = _write(fh,
			(char *)&hdr,
			sizeof(PF_FileHdr));
		if (numBytes < 0)
			return (PF_UNIX);
		if (numBytes != sizeof(PF_FileHdr))
			return (PF_HDRWRITE);
		PF_FileHandle *dummy = (PF_FileHandle *)this;
		dummy->bHdrChanged = FALSE;
	}
	return (pBufferMgr->FlushPages(fh));
}

RC PF_FileHandle::ForcePages(PageNum pageNum) const
{
	if (!bFileOpen)
		return (PF_CLOSEDFILE);
	if (bHdrChanged) {
		if (_lseek(fh, 0, L_SET) < 0)
			return (PF_UNIX);
		int numBytes = _write(fh,
			(char *)&hdr,
			sizeof(PF_FileHdr));
		if (numBytes < 0)
			return (PF_UNIX);
		if (numBytes != sizeof(PF_FileHdr))
			return (PF_HDRWRITE);
		PF_FileHandle *dummy = (PF_FileHandle *)this;
		dummy->bHdrChanged = FALSE;
	}

	return (pBufferMgr->ForcePages(fh, pageNum));
}

int PF_FileHandle::IsValidPageNum(PageNum pageNum) const
{
	return (bFileOpen &&
		pageNum >= 0 &&
		pageNum < hdr.numPages);
}