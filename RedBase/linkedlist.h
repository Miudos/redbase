#pragma once
#ifndef LINKEDLIST_H
	#define LINKEDLIST_H

	#include "redbase.h" //required to compile successfully statistics.h

	template <class T>
	class LinkList
	{
	public:
		LinkList();
		LinkList(const LinkList<T> & sourcell);
		~LinkList();
		void operator = (const LinkList<T> & sourcell);

		Boolean operator == (const LinkList<T> & rhs) const;
		operator T * ();

		void Append(const T & item);
		void Append(const LinkList<T> & sourcell);

		LinkList<T> operator+(const LinkList<T> &sourcell) const;
		LinkList<T> operator+(const T &element) const;
		void operator+=(const LinkList<T> &sourcell);
		void operator+=(const T &element);

		void Delete(int index);
		void Erase();

		// 0..(1-GetLength())
		T* Get(int index);
		T* operator[](int index);
		int  GetLength() const;
	protected:
		struct InternalNode {
			T Data;
			InternalNode * next;
			InternalNode * previous;
		};

		int iLength;
		InternalNode *pnHead;
		InternalNode *pnTail;
		InternalNode *pnLastRef;
		int iLastRef;
								
		void SetNull();
	};

	template <class T>
	inline void LinkList<T>::SetNull()
	{
		pnHead = NULL;
		pnTail = NULL;
		pnLastRef = NULL;
		iLength = 0;
		iLastRef = -1;
	}

	template <class T>
	inline LinkList<T>::LinkList()
	{
		SetNull();
	}

	template <class T>
	LinkList<T>::LinkList(const LinkList<T> & sourcell)
	{
		SetNull();

		if (sourcell.iLength == 0)
			return;

		InternalNode *n = sourcell.pnHead;

		while (n != NULL)
		{
			Append(n->Data);
			n = n->next;
		}
		pnLastRef = pnHead;
	}

	template <class T>
	inline LinkList<T>::~LinkList()
	{
		Erase();
	}

	template <class T>
	void LinkList<T>::operator = (const LinkList<T> & sourcell)
	{
		Erase();
		InternalNode *pnTemp = sourcell.pnHead;

		while (pnTemp != NULL)
		{
			Append(pnTemp->Data);
			pnTemp = pnTemp->next;
		}

		pnLastRef = NULL;
		iLastRef = -1;
	}

	template <class T>
	Boolean LinkList<T>::operator == (const LinkList<T> & rhs) const
	{
		if (iLength != rhs.iLength)
			return (FALSE);

		InternalNode *pnLhs = this->pnHead;
		InternalNode *pnRhs = rhs.pnHead;

		while (pnLhs != NULL && pnRhs != NULL)
		{
			if (!(pnLhs->Data == pnRhs->Data))
				return FALSE;
			pnLhs = pnLhs->next;
			pnRhs = pnRhs->next;
		}

		if (pnLhs == NULL && pnRhs == NULL)
			return TRUE;
		else
			return FALSE;
	}

	template <class T>
	LinkList<T>::operator T * ()
	{
		if (iLength == 0)
			return NULL;

		T *pResult = new T[iLength];

		InternalNode *pnCur = pnHead;
		T *pnCopy = pResult;

		while (pnCur != NULL)
		{
			*pnCopy = pnCur->Data;
			++pnCopy;
			pnCur = pnCur->next;
		}
		return pResult;
	}

	template <class T>
	inline void LinkList<T>::Append(const T & item)
	{
		InternalNode *pnNew = new InternalNode;

		pnNew->Data = item;
		pnNew->next = NULL;
		pnNew->previous = pnTail;

		if (iLength == 0)
		{
			pnHead = pnNew;
			pnTail = pnNew;
			pnLastRef = pnNew;
		}
		else
		{
			pnTail->next = pnNew;
			pnTail = pnNew;
		}
		++iLength;
	}

	template <class T>
	inline LinkList<T>
	LinkList<T>::operator+(const LinkList<T> &sourcell) const
	{
		LinkList<T> pTempLL(*this);
		pTempLL += sourcell;
		return pTempLL;
	}

	template <class T>
	inline LinkList<T>
	LinkList<T>::operator+(const T &element) const
	{
		LinkList<T> pTempLL(*this);
		pTempLL += element;
		return pTempLL;
	}

	template <class T>
	void
	LinkList<T>::operator+=(const LinkList<T> &list)
	{
		const InternalNode *pnTemp;
		const int iLength = list.iLength;
		int i;

		for (pnTemp = list.pnHead, i = 0; i < iLength; pnTemp = pnTemp->next, i++)
			*this += pnTemp->Data;
	}

	template <class T>
	void
	LinkList<T>::operator+=(const T &element)
	{
		InternalNode *pnNew = new InternalNode;
		pnNew->next = NULL;
		pnNew->Data = element;
		if (iLength++ == 0) {
			pnHead = pnNew;
			pnNew->previous = NULL;
		}
		else {
			pnTail->next = pnNew;
			pnNew->previous = pnTail;
		}
		pnTail = pnNew;
	}

	template <class T>
	void LinkList<T>::Append(const LinkList<T> & sourcell)
	{
		const InternalNode *pnCur = sourcell.pnHead;

		while (pnCur != NULL)
		{
			Append(pnCur->Data);
			pnCur = pnCur->next;
		}
	}

	template <class T>
	inline void LinkList<T>::Delete(int which)
	{
		if (which>iLength || which == 0)
			return;

		InternalNode *pnDeleteMe = pnHead;

		for (int i = 1; i<which; i++)
			pnDeleteMe = pnDeleteMe->next;

		if (pnDeleteMe == pnHead)
		{
			if (pnDeleteMe->next == NULL)
			{
				delete pnDeleteMe;
				SetNull();
			}
			else
			{
				pnHead = pnDeleteMe->next;
				pnHead->previous = NULL;
				delete pnDeleteMe;
				pnLastRef = pnHead;
			}
		}
		else
		{
			if (pnDeleteMe == pnTail)
			{
				if (pnDeleteMe->previous == NULL)
				{
					delete pnDeleteMe;
					SetNull();
				}
				else
				{
					pnTail = pnDeleteMe->previous;
					pnTail->next = NULL;
					delete pnDeleteMe;
					pnLastRef = pnTail;
				}
			}
			else
			{
				pnLastRef = pnDeleteMe->next;
				pnDeleteMe->previous->next = pnDeleteMe->next;
				pnDeleteMe->next->previous = pnDeleteMe->previous;
				delete pnDeleteMe;
			}
		}

		if (iLength != 0)
			--iLength;
	}

	template <class T>
	inline T* LinkList<T>::operator[](int index)
	{
		return (Get(index));
	}

	template <class T>
	inline void LinkList<T>::Erase()
	{
		pnLastRef = pnHead;

		while (pnLastRef != NULL)
		{
			pnHead = pnLastRef->next;
			delete pnLastRef;
			pnLastRef = pnHead;
		}
		SetNull();
	}

	template <class T>
	inline T* LinkList<T>::Get(int index)
	{
		int iCur;
		InternalNode *pnTemp;
		int iRelToMiddle;
		if (index < 0 || index >= iLength)
			return NULL;

		index++;

		if (iLastRef == -1)
			if (index < (iLength - index)) {
				iCur = 1;
				pnTemp = pnHead;
			}
			else {
				iCur = iLength;
				pnTemp = pnTail;
			}
		else
		{
			if (index < iLastRef)
				iRelToMiddle = iLastRef - index;
			else
				iRelToMiddle = index - iLastRef;

			if (index < iRelToMiddle) {
				iCur = 1;
				pnTemp = pnHead;
			}
			else
				if (iRelToMiddle < (iLength - index)) {
					iCur = iLastRef;
					pnTemp = pnLastRef;
				}
				else {
					iCur = iLength;
					pnTemp = pnTail;
				}
		}

		while (iCur != index)
			if (iCur < index) {
				iCur++;
				pnTemp = pnTemp->next;
			}
			else {
				iCur--;
				pnTemp = pnTemp->previous;
			}

			iLastRef = index;
			pnLastRef = pnTemp;

			return &(pnLastRef->Data);
	}

	template <class T>
	inline int LinkList<T>::GetLength() const
	{
		return iLength;
	}
#endif // !LINKEDLIST_H
