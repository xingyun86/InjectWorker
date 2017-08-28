// aboutdlg.h : interface of the CAboutDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

class CAboutDlg : public CDialogImpl<CAboutDlg>
{
public:
	enum { IDD = IDD_DIALOG_ABOUTBOX };

	BEGIN_MSG_MAP(CAboutDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER_EX(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(HWND, LPARAM)
	{
		CenterWindow(GetParent());
		return TRUE;
	}

	LRESULT OnCloseCmd(UINT, int nID, HWND)
	{
		EndDialog(nID);
		return 0;
	}
};
