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

#include "DialogItemTemplate.h"

// DLGTEMPLATEEX is not explicitly defined by Windows, so we borrow the ATL definition.
//typedef _DialogSplitHelper::DLGTEMPLATEEX __DSH_DLGTEMPLATEEX;
//typedef _DialogSplitHelper::DLGITEMTEMPLATEEX __DSH_DLGITEMTEMPLATEEX;



// Wrapper class around a DLGTEMPLATE(EX)
class CDialogTemplate
{
private:

	bool m_bIsTmplEx;		// true if this is a ...EX template

	union
	{
		__DSH_DLGTEMPLATE* m_pTmpl;
		__DSH_DLGTEMPLATEEX* m_pTmplEx;
	};

	mutable CSize m_cxyBaseUnits;
	static HINSTANCE s_hResourceInstance;	// instance handle for loading resources

public:

	CDialogTemplate()
		: m_pTmpl(NULL), m_cxyBaseUnits(0, 0) { }

	CDialogTemplate(UINT nResourceID)
		: m_pTmpl(NULL), m_cxyBaseUnits(0, 0)
	{
		LoadFromResource(nResourceID);
	}


	// Sets the resource instance handle. Use this if your resources are located in another
	// instance than your main instance.
	static void SetResourceInstance(HINSTANCE hResourceInstance)
	{
		s_hResourceInstance = hResourceInstance;
	}


	/*
	 * Loads a dialog template from a resource.
	 * Resources are loaded from the instance specified by SetResourceInstance().
	 */
	void LoadFromResource(UINT nResourceID);


	// Returns the x (left) position of the dialog, in DLUs.
	short GetX() const
	{
		ATLASSERT( m_pTmpl != NULL );
		return m_bIsTmplEx ? m_pTmplEx->x : m_pTmpl->x;
	}


	// Returns the y (top) position of the dialog, in DLUs.
	short GetY() const
	{
		ATLASSERT( m_pTmpl != NULL );
		return m_bIsTmplEx ? m_pTmplEx->y : m_pTmpl->y;
	}


	// Returns the width of the dialog, in DLUs.
	short GetWidth() const
	{
		ATLASSERT( m_pTmpl != NULL );
		return m_bIsTmplEx ? m_pTmplEx->cx : m_pTmpl->cx;
	}


	// Returns the height of the dialog, in DLUs.
	short GetHeight() const
	{
		ATLASSERT( m_pTmpl != NULL );
		return m_bIsTmplEx ? m_pTmplEx->cy : m_pTmpl->cy;
	}


	// Returns the window style of the dialog.
	DWORD GetStyle() const
	{
		ATLASSERT( m_pTmpl != NULL );
		return m_bIsTmplEx ? m_pTmplEx->style : m_pTmpl->style;
	}


	// Returns the extended window style of the dialog.
	DWORD GetExtendedStyle() const
	{
		ATLASSERT( m_pTmpl != NULL );
		return m_bIsTmplEx ? m_pTmplEx->exStyle : m_pTmpl->dwExtendedStyle;
	}


	// Returns the number of dialog items contained in the dialog.
	WORD GetItemCount() const
	{
		ATLASSERT( m_pTmpl != NULL );
		return m_bIsTmplEx ? m_pTmplEx->cDlgItems : m_pTmpl->cdit;
	}


	// Returns the resource identifier of the dialog's menu
	// (or NULL if the dialog has no menu).
	LPCWSTR GetMenuResource() const;

	// Returns the window class name of the dialog.
	LPCWSTR GetClassName() const;

	// Returns the window title of the dialog.
	LPCWSTR GetTitle() const;

	// Retrieves the font info for the dialog.
	BOOL GetFontInfo(LOGFONTW& lf) const;

	// Returns the first dialog item.
	// If the dialog does not contain any items, returns an invalid CDialogTemplate.
	CDialogItemTemplate GetFirstDlgItem() const;

	// Finds the dialog item with the given ID.
	// If there is no such item, returns an invalid CDialogItemTemplate.
	CDialogItemTemplate FindControl(UINT nCtrlID) const;

	/* Convert a rectangle from dialog box units to screen units (pixels).
	 * This does the same as Windows's MapDialogRect function, but works on the
	 * template and thus doesn't need a window handle. */
	BOOL MapDialogRect(LPRECT pRect) const;

private:

	// Returns a pointer to the data immediately following the template header.
	const WORD* GetDataPtr() const;
	
	// Skips a variable-sized data field following the header.
	static void SkipData(const WORD*& pw);

	// Skips the title field.
	static void SkipTitle(const WORD*& pw);

	// Skips the font info.
	void SkipFontInfo(const WORD*& pw) const;
};
