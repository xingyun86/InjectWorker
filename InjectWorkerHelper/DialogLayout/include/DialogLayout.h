/* DialogLayout
 * Copyright (C) Till Krullmann.
 *
 * The use and distribution terms for this software are covered by the
 * Common Public License 1.0 (http://opensource.org/licenses/cpl.php)
 * which can be found in the file CPL.TXT at the root of this distribution.
 * By using this software in any fashion, you are agreeing to be bound by
 * the terms of this license. You must not remove this notice, or
 * any other, from this software.
 * 
 * 
 * FILE: DialogLayout.h - Automatic layouting of WTL dialogs
 *
 * This file rovides several macros to simplify automatic layouting for WTL dialogs.
 * To add automatic layout to your dialog class, follow these steps:
 * 1. Derive your class from CDialogLayout<T>
 * 2. Define a "layout map" using the LAYOUT_MAP macros
 * 3. Add CHAIN_MSG_MAP(CDialogLayout<T>) to your message map
 * 4. If you handle WM_SIZE or WM_INITDIALOG yourself, be sure to call 
 *        SetMsgHandled(FALSE) in your handlers.
 */



#pragma once

#include "DialogTemplate.h"



// "breaks" a RECT in left, top, width, height
#define RECT_BREAK(rect)	rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top



#define LAYOUT_ANCHOR_NONE			0x0000U
#define LAYOUT_ANCHOR_LEFT			0x0001U
#define LAYOUT_ANCHOR_TOP			0x0002U
#define LAYOUT_ANCHOR_RIGHT			0x0004U
#define LAYOUT_ANCHOR_BOTTOM		0x0008U
#define LAYOUT_ANCHOR_TOP_LEFT		(LAYOUT_ANCHOR_TOP | LAYOUT_ANCHOR_LEFT)
#define LAYOUT_ANCHOR_BOTTOM_LEFT	(LAYOUT_ANCHOR_BOTTOM | LAYOUT_ANCHOR_LEFT)
#define LAYOUT_ANCHOR_TOP_RIGHT		(LAYOUT_ANCHOR_TOP | LAYOUT_ANCHOR_RIGHT)
#define LAYOUT_ANCHOR_BOTTOM_RIGHT	(LAYOUT_ANCHOR_BOTTOM | LAYOUT_ANCHOR_RIGHT)
#define LAYOUT_ANCHOR_HORIZONTAL	(LAYOUT_ANCHOR_LEFT | LAYOUT_ANCHOR_RIGHT)
#define LAYOUT_ANCHOR_VERTICAL		(LAYOUT_ANCHOR_TOP | LAYOUT_ANCHOR_BOTTOM)
#define LAYOUT_ANCHOR_ALL			(LAYOUT_ANCHOR_HORIZONTAL | LAYOUT_ANCHOR_VERTICAL)


#define LAYOUT_DIRECTION_LEFT				0x0001
#define LAYOUT_DIRECTION_TOP				0x0002
#define LAYOUT_DIRECTION_RIGHT				0x0004
#define LAYOUT_DIRECTION_BOTTOM				0x0008



///////////////////////////////////////////////////////////////////////////////
// CLayoutRule

/* Layout rules define how the 4 sides of a container are positioned in the dialog.
 * CLayoutRule is an abstract base class for the different types of rules.
 */

class ATL_NO_VTABLE CLayoutRule
{
public:
	// Applies this rule to a dialog template for determining the initial bounds.
	virtual LONG Apply(const CDialogTemplate&, UINT nDirection, const CRect& rcLayout) const = 0;

	// Applies this rule and returns the actual coordinate.
	virtual LONG Apply(CWindow wndLayout, UINT nDirection, const CRect& rcLayout) const = 0;
};


typedef CAutoPtr<CLayoutRule> CLayoutRulePtr;
typedef CAutoPtrArray<CLayoutRule> CLayoutRulePtrArray;



///////////////////////////////////////////////////////////////////////////////
// CLayoutRuleAbsolute

/* CLayoutRuleAbsolute takes absolute coordinates (in DLUs) for positioning. Coordinates
 * can be negative; in this case they will start from the right/bottom side. */

class CLayoutRuleAbsolute : public CLayoutRule
{
private:
	LONG m_nPos;
public:

	CLayoutRuleAbsolute(LONG nPos)
		: m_nPos(nPos)
	{ }

	LONG GetPosition() const { return m_nPos; }

	LONG Apply(const CDialogTemplate&, UINT nDirection, const CRect& rcLayout) const;
	LONG Apply(CWindow wndLayout, UINT nDirection, const CRect& rcLayout) const;
};


///////////////////////////////////////////////////////////////////////////////
// CLayoutRuleRelativeControl

/* CLayoutRuleRelativeControl aligns the edge of a container to the edge of a control
 * (with some optional padding). The control must have been declared in the layout map
 * before the container. 
 */

class CLayoutRuleRelativeControl : public CLayoutRule
{
private:
	UINT m_nCtrlID;
	UINT m_nDirection;
	UINT m_nPadding;

public:

	CLayoutRuleRelativeControl(UINT nCtrlID, UINT nDirection, UINT nPadding=0)
		: m_nCtrlID(nCtrlID), m_nDirection(nDirection), m_nPadding(nPadding)
	{ }

	UINT GetControlID() const { return m_nCtrlID; }
	UINT GetDirection() const { return m_nDirection; }

	LONG Apply(const CDialogTemplate&, UINT nDirection, const CRect& rcLayout) const;
	LONG Apply(CWindow wndLayout, UINT nDirection, const CRect& rcLayout) const;
};



///////////////////////////////////////////////////////////////////////////////
// CLayoutRuleRatio

/* CLayoutRuleRatio causes a container to always occupy a part (say, 50%) of the
 * parent container's size.
 */

class CLayoutRuleRatio : public CLayoutRule
{
private:
	float m_fRatio;

public:
	CLayoutRuleRatio(float fRatio)
		: m_fRatio(fRatio)
	{
		ATLASSERT( fRatio >= 0.0f && fRatio <= 1.0f );
	}

	LONG Apply(const CDialogTemplate&, UINT nDirection, const CRect& rcLayout) const;
	LONG Apply(CWindow wndLayout, UINT nDirection, const CRect& rcLayout) const;
};



///////////////////////////////////////////////////////////////////////////////
// CLayoutNode

/*
 * CLayoutNode is the abstract base class for layout containers and layout controls.
 */

class ATL_NO_VTABLE CLayoutNode
{
public:
	virtual void DoLayout(CWindow wndLayout, HDWP hDwp, const CRect& rcLayout) = 0;
};


typedef CAutoPtr<CLayoutNode> CLayoutNodePtr;
typedef CAutoPtrList<CLayoutNode> CLayoutNodePtrList;



///////////////////////////////////////////////////////////////////////////////
// CLayoutContainer

/*
 * CLayoutContainer represents a rectangular area in the dialog used for
 * layouting. The four sides of this rectangle are defined by layout rules; the
 * container does not have to be attached to any actual window.
 * CLayoutContainer can contain control layouts as well as other layout containers.
 * It defines the reference layout area for the layout nodes it contains.
 */

class CLayoutContainer : public CLayoutNode
{
private:
	const CDialogTemplate& m_rTmpl;	// reference to the parent dialog template
	CRect m_rcBounds;				// initial bounds on dialog template
	CLayoutRulePtrArray m_Rules;	// 4 rules for left, top, right, bottom
	CLayoutNodePtrList m_LayoutList;

public:

	CLayoutContainer(const CDialogTemplate& rTmpl, const CLayoutContainer* pParent=NULL,
		CLayoutRule* pLeftRule=NULL, CLayoutRule* pTopRule=NULL,
		CLayoutRule* pRightRule=NULL, CLayoutRule* pBottomRule=NULL );

public:

	size_t GetEntryCount() const { return m_LayoutList.GetCount(); }
	virtual void AddLayoutNode(CLayoutNode* pNode);
	const CRect& GetBounds() const { return m_rcBounds; }

protected:
	const CLayoutNodePtrList& GetLayoutList() const { return m_LayoutList; }

	void CalcRect(HWND hwndLayout, const CRect& rcLayout);

public:
	void DoLayout(CWindow wndLayout, HDWP hDwp, const CRect& rcLayout);
};



///////////////////////////////////////////////////////////////////////////////
// CLayoutControl

/*
 * CLayoutControl is used to layout a control, given its control ID.
 * It stores the anchor flags and the margins used for layouting.
 */

class CLayoutControl : public CLayoutNode
{
private:
	
	UINT m_nID;
	UINT m_nAnchor;
	CRect m_rcMargins;	

public:

	CLayoutControl(const CDialogTemplate&, const CLayoutContainer& parent, UINT nID,
		UINT nAnchor = LAYOUT_ANCHOR_NONE);

	UINT GetID() const { return m_nID; }
	UINT GetAnchor() const { return m_nAnchor; }
	const CRect& GetMargins() const { return m_rcMargins; }

private:

	HWND CalcRect(CWindow wndLayout, LPCRECT prcLayout, LPRECT prc);

public:
	void DoLayout(CWindow wndLayout, HDWP hDwp, const CRect& rcLayout);
};



///////////////////////////////////////////////////////////////////////////////
// CDialogLayout

/*
 * CDialogLayout is an abstract base class for dialog classes that use dialog layouting.
 */


extern ATOM g_atomPropHDWP;



template <class T>
class ATL_NO_VTABLE CDialogLayout : public CMessageMap
{
public:

	BEGIN_MSG_MAP(CDialogLayout)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
	END_MSG_MAP()

private:
	static ATOM s_atomButton;

protected:
	CDialogTemplate m_Tmpl;
	CLayoutContainer* m_pRootContainer;
	BOOL m_bGripper;
	

	CDialogLayout()
		:  m_Tmpl(T::IDD), m_pRootContainer(NULL), m_bGripper(TRUE)
	{
		if (s_atomButton == 0)
		{
			WNDCLASS wc;
			s_atomButton = (ATOM) GetClassInfo( NULL, WC_BUTTON, &wc );
		}
	}

	~CDialogLayout()
	{
		delete m_pRootContainer;
	}


	void DoLayout(CLayoutContainer* pContainer, const CRect& rcLayout)
	{
		ATLASSERT( pContainer != NULL );

		T* pT = static_cast<T*>(this);

		HDWP hDwp = BeginDeferWindowPos( (int) pContainer->GetEntryCount() );
		SetProp( pT->m_hWnd, MAKEINTATOM(g_atomPropHDWP), (HANDLE) hDwp );
		pContainer->DoLayout( pT->m_hWnd, hDwp, rcLayout );

		// If there is a gripper in the corner, adjust its size, too
		if ( m_bGripper )
		{
			T* pT = static_cast<T*>(this);
			CWindow m_wndGripper = pT->GetDlgItem(ATL_IDW_STATUS_BAR);
			ATLASSERT( ::IsWindow(m_wndGripper) );
			
			CRect rcGripperRect;
			m_wndGripper.GetWindowRect(rcGripperRect);
			
			m_wndGripper.DeferWindowPos( hDwp, NULL, rcLayout.right - rcGripperRect.Width(),
				rcLayout.bottom - rcGripperRect.Height(), 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER );
		}

		EndDeferWindowPos(hDwp);

		/* This fixes a strange Windows bug which makes group boxes disappear.
		 * Check all the child controls, and if they are a group box, bring them
		 * to Z-front and redraw them...
		 */
		CSimpleArray<HWND> groupBoxes;
		for ( CWindow wndChild = pT->GetWindow(GW_CHILD); (HWND) wndChild;
			wndChild = wndChild.GetWindow(GW_HWNDNEXT) )
		{
			bool bIsGroupBox = ( (ATOM) GetClassLong( wndChild, GCW_ATOM ) == s_atomButton )
				&& ( (wndChild.GetStyle() & BS_GROUPBOX) == BS_GROUPBOX );
			if ( bIsGroupBox )
				groupBoxes.Add(wndChild.m_hWnd);				
		}

		for ( int i = 0; i < groupBoxes.GetSize(); ++i )
		{
			::SetWindowPos( groupBoxes[i], HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE
				| SWP_DRAWFRAME );
		}
	}

private:

	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL& bHandled)
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT( ::IsWindow(pT->m_hWnd) );

		m_pRootContainer = new CLayoutContainer(m_Tmpl);
		pT->SetupLayout();

		/* This is pretty much copied from CDialogResize. */
		if ( m_bGripper )
		{
			ATLASSERT( !::IsWindow( pT->GetDlgItem(ATL_IDW_STATUS_BAR) ) );
			if ( !::IsWindow( pT->GetDlgItem(ATL_IDW_STATUS_BAR) ) )
			{
				RECT rcClient;
				pT->GetClientRect(&rcClient);

				CWindow wndGripper;
				const DWORD dwGripperStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS
					| SBS_SIZEGRIP | SBS_SIZEBOXBOTTOMRIGHTALIGN;
				wndGripper.Create( _T("SCROLLBAR"), pT->m_hWnd, rcClient, NULL, dwGripperStyle, 0,
					ATL_IDW_STATUS_BAR );
			}
		}

		bHandled = FALSE;
		return 0;
	}


	LRESULT OnSize(UINT, WPARAM, LPARAM lParam, BOOL& bHandled)
	{
		DoLayout( m_pRootContainer, CRect(0, 0, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)) );

		bHandled = FALSE;
		return 0;
	}
};


template <class T>
ATOM CDialogLayout<T>::s_atomButton = 0;



#define ABS(coord)		new CLayoutRuleAbsolute(coord)

#define LEFT_OF(ctrlID)		\
	new CLayoutRuleRelativeControl(ctrlID, LAYOUT_DIRECTION_LEFT)
#define ABOVE(ctrlID)		\
	new CLayoutRuleRelativeControl(ctrlID, LAYOUT_DIRECTION_TOP)
#define RIGHT_OF(ctrlID)	\
	new CLayoutRuleRelativeControl(ctrlID, LAYOUT_DIRECTION_RIGHT)
#define BELOW(ctrlID)		\
	new CLayoutRuleRelativeControl(ctrlID, LAYOUT_DIRECTION_BOTTOM)
#define LEFT_OF_PAD(ctrlID, padding)	\
	new CLayoutRuleRelativeControl(ctrlID, LAYOUT_DIRECTION_LEFT, padding)
#define ABOVE_PAD(ctrlID, padding)		\
	new CLayoutRuleRelativeControl(ctrlID, LAYOUT_DIRECTION_TOP, padding)
#define RIGHT_OF_PAD(ctrlID, padding)	\
	new CLayoutRuleRelativeControl(ctrlID, LAYOUT_DIRECTION_RIGHT, padding)
#define BELOW_PAD(ctrlID, padding)		\
	new CLayoutRuleRelativeControl(ctrlID, LAYOUT_DIRECTION_BOTTOM, padding)

#define BEGIN_LAYOUT_CONTAINER_AROUND_CONTROL(ctrlID) \
	BEGIN_LAYOUT_CONTAINER( LEFT_OF(ctrlID), ABOVE(ctrlID), RIGHT_OF(ctrlID), BELOW(ctrlID) )

#define RATIO(ratio)	new CLayoutRuleRatio((float) ratio)



#define BEGIN_LAYOUT_MAP() \
void SetupLayout() \
{ \
	CAtlList<CLayoutContainer*> containers;	\
	containers.AddTail(m_pRootContainer); \
	CLayoutContainer* pCurrentContainer = m_pRootContainer;


#define LAYOUT_CONTROL(ID, anchor) \
	pCurrentContainer->AddLayoutNode( new CLayoutControl( m_Tmpl, *pCurrentContainer, (ID), (anchor) ) ); \


#define END_LAYOUT_MAP() \
	containers.RemoveTailNoReturn(); \
	ATLASSERT( containers.IsEmpty() ); \
} \

#define BEGIN_LAYOUT_CONTAINER(leftRule, topRule, rightRule, bottomRule) \
{ \
	CLayoutContainer* pNewContainer = new CLayoutContainer( m_Tmpl, pCurrentContainer,\
		leftRule, topRule, rightRule, bottomRule ); \
	pCurrentContainer->AddLayoutNode(pNewContainer); \
	containers.AddTail(pNewContainer); \
	pCurrentContainer = pNewContainer; \
} \


#define END_LAYOUT_CONTAINER() \
	containers.RemoveTailNoReturn(); \
	pCurrentContainer = containers.GetTail();
