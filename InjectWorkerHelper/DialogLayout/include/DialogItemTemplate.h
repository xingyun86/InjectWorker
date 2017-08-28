/* DialogLayout
 * Copyright (C) Till Krullmann.
 *
 * The use and distribution terms for this software are covered by the
 * Common Public License 1.0 (http://opensource.org/licenses/cpl.php)
 * which can be found in the file CPL.TXT at the root of this distribution.
 * By using this software in any fashion, you are agreeing to be bound by
 * the terms of this license. You must not remove this notice, or
 * any other, from this software.
 */

#pragma once

#include <atlbase.h>
#include <atlwin.h>
#include <atltypes.h>
#include <atlcoll.h>

// DLGITEMTEMPLATEEX is not explicitly defined by Windows, so we borrow the ATL definition.
typedef ::DLGTEMPLATE __DSH_DLGTEMPLATE;
typedef ::DLGITEMTEMPLATE __DSH_DLGITEMTEMPLATE;
typedef _DialogSplitHelper::DLGTEMPLATEEX __DSH_DLGTEMPLATEEX;
typedef _DialogSplitHelper::DLGITEMTEMPLATEEX __DSH_DLGITEMTEMPLATEEX;



// Aligns a pointer to the next 4-byte (DWORD) boundary.
template <class T>
inline DWORD_PTR AlignToDWORD(T* p)
{
	return ( (DWORD_PTR) p + 3 ) & ~3;
}



// Forward declaration
class CDialogTemplate;



/*
 * Wrapper class around a DLGITEMTEMPLATE(EX).
 * CDialogItemTemplate´s are essentially pointers, so they are passed around by value
 * here.
 */
class CDialogItemTemplate
{
private:
	
	const CDialogTemplate& m_rParent;	// reference to the parent dialog template
	bool m_bIsItemEx;					// true if this is a ...EX template

	union
	{
		const __DSH_DLGITEMTEMPLATE* m_pItem;
		const __DSH_DLGITEMTEMPLATEEX* m_pItemEx;
	};


	CDialogItemTemplate(const CDialogTemplate& rParent, const DLGITEMTEMPLATE* pItem,
		bool bIsItemEx)
		: m_rParent(rParent), m_pItem(pItem), m_bIsItemEx(bIsItemEx)
	{ }

	friend class CDialogTemplate;


public:

	CDialogItemTemplate(const CDialogItemTemplate& item)
		: m_rParent(item.m_rParent), m_pItem(item.m_pItem), m_bIsItemEx(item.m_bIsItemEx)
	{ }

	
	CDialogItemTemplate& operator=(const CDialogItemTemplate& item)
	{
		ATLASSERT( &m_rParent == &item.m_rParent );
		m_pItem = item.m_pItem;
		return *this;
	}


	// Returns the parent CDialogTemplate (the dialog which this control is contained in).
    const CDialogTemplate& GetParent() const
	{
		return m_rParent;
	}


	// Checks whether the template pointer is valid (non-null).
	bool IsValid() const
	{
		return m_pItem != NULL;
	}


	// Returns the x (left) position of the dialog item, in DLUs.
	short GetX() const
	{
		ATLASSERT( m_pItem != NULL );
		return m_bIsItemEx ? m_pItemEx->x : m_pItem->x;
	}


	// Returns the y (top) position of the dialog item, in DLUs.
	short GetY() const
	{
		ATLASSERT( m_pItem != NULL );
		return m_bIsItemEx ? m_pItemEx->y : m_pItem->y;
	}


	// Returns the width of the dialog item, in DLUs.
	short GetWidth() const
	{
		ATLASSERT( m_pItem != NULL );
		return m_bIsItemEx ? m_pItemEx->cx : m_pItem->cx;
	}


	// Returns the height of the dialog item, in DLUs.
	short GetHeight() const
	{
		ATLASSERT( m_pItem != NULL );
		return m_bIsItemEx ? m_pItemEx->cy : m_pItem->cy;
	}


	// Returns the identifier of the dialog item.
	DWORD GetID() const
	{
		ATLASSERT( m_pItem != NULL );
		return m_bIsItemEx ? m_pItemEx->id : m_pItem->id;
	}


	// Returns the window style of the dialog item.
	DWORD GetStyle() const
	{
		ATLASSERT( m_pItem != NULL );
		return m_bIsItemEx ? m_pItemEx->style : m_pItem->style;
	}


	// Returns the extended window style of the dialog item.
	DWORD GetExtendedStyle() const
	{
		ATLASSERT( m_pItem != NULL );
		return m_bIsItemEx ? m_pItemEx->exStyle : m_pItem->dwExtendedStyle;
	}


	// Returns the help context ID of the dialog item.
	DWORD GetHelpID() const
	{
		ATLASSERT( m_pItem != NULL );
		return m_bIsItemEx ? m_pItemEx->helpID : 0;
	}

	// Returns the window class name of the dialog item.
	// Note that this is always stored as a UNICODE string in the template.
	LPCWSTR GetClassName() const;

	// Returns the initial window text of the dialog item.
	// Note that this is always stored as a UNICODE string in the template.
	LPCWSTR GetTitle() const;

	// Returns the next item in the dialog.
	CDialogItemTemplate GetNextItem() const;

private:

	// Returns a pointer to the data immediately following the template header.
	const WORD* GetDataPtr() const
	{
		if ( m_bIsItemEx )
			return (const WORD*) (m_pItemEx + 1);
		else
			return (const WORD*) (m_pItem + 1);
	}

	// Skips a variable-sized data field following the header (windowClass or title)
	static void SkipData(const WORD*& pw);

	// Skips the extra data field following the header.
	static void SkipExtraData(const WORD*& pw, bool bDialogEx);
};
