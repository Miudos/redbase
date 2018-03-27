#pragma region Includes
	#include "pf_internal.h"
#pragma endregion

#define INVALID_PAGE   (-1)

PF_PageHandle::PF_PageHandle()
{
	pageNum = INVALID_PAGE;
	pPageData = NULL;
}

PF_PageHandle::~PF_PageHandle(){}

PF_PageHandle::PF_PageHandle(const PF_PageHandle &pageHandle)
{
	this->pageNum = pageHandle.pageNum;
	this->pPageData = pageHandle.pPageData;
}

PF_PageHandle& PF_PageHandle::operator= (const PF_PageHandle &pageHandle)
{
	if (this != &pageHandle) {

		this->pageNum = pageHandle.pageNum;
		this->pPageData = pageHandle.pPageData;
	}
	return (*this);
}

RC PF_PageHandle::GetData(char *&pData) const
{
	if (pPageData == NULL)
		return (PF_PAGEUNPINNED);

	pData = pPageData;
	return (0);
}

RC PF_PageHandle::GetPageNum(PageNum &_pageNum) const
{
	if (pPageData == NULL)
		return (PF_PAGEUNPINNED);
	_pageNum = this->pageNum;
	return (0);
}
