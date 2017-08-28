// RegistryInjectDlg.h : interface of the CRegistryInjectDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CodeDataBase.h"
#include <DialogLayout.h>


class CRegistryInjectDlg :
	public CCodeDataBase,
	public CDialogImpl<CRegistryInjectDlg>,
	public CDialogLayout<CRegistryInjectDlg>,
	public CUpdateUI<CRegistryInjectDlg>,
	public CMessageFilter,
	public CIdleHandler
{
public:
	enum { IDD = IDD_DIALOG_REGISTRYINJECT };

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		return CWindow::IsDialogMessage(pMsg);
	}

	virtual BOOL OnIdle()
	{
		UIUpdateChildWindows();
		return FALSE;
	}

	BEGIN_UPDATE_UI_MAP(CRegistryInjectDlg)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CRegistryInjectDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_SYSCOMMAND(OnSysCommand)
		MSG_WM_NOTIFY(OnNotify)
		COMMAND_ID_HANDLER_EX(IDC_BUTTON_SELECTAPPEXE, OnSelectAppExe)
		COMMAND_ID_HANDLER_EX(IDC_BUTTON_SELECTAPPDLL, OnSelectAppDll)
		COMMAND_ID_HANDLER_EX(IDC_BUTTON_SUSPENDPROCESSINJECT, OnSuspendProcessInject)
		COMMAND_ID_HANDLER_EX(IDC_BUTTON_RELEASEMEMORYSPACES, OnReleaseMemorySpaces)
		COMMAND_ID_HANDLER_EX(ID_APP_ABOUT, OnAppAbout)
		COMMAND_ID_HANDLER_EX(IDOK, OnOK)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnCancel)
		CHAIN_MSG_MAP(CDialogLayout<CRegistryInjectDlg>)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	BEGIN_LAYOUT_MAP()
		//BEGIN_LAYOUT_CONTAINER(ABS(0.0), ABS(0.0), RATIO(1.0), RATIO(0.5))
		//LAYOUT_CONTROL(IDC_LIST_PROCESSES, LAYOUT_ANCHOR_ALL)
		//END_LAYOUT_CONTAINER()
		//BEGIN_LAYOUT_CONTAINER(ABS(0.0), RATIO(0.5), RATIO(1.0), RATIO(1.0))
		//LAYOUT_CONTROL(IDC_LIST_MODULES, LAYOUT_ANCHOR_ALL)
		//END_LAYOUT_CONTAINER()
		//LAYOUT_CONTROL(IDOK, LAYOUT_ANCHOR_RIGHT | LAYOUT_ANCHOR_BOTTOM)
		//LAYOUT_CONTROL(IDCANCEL, LAYOUT_ANCHOR_RIGHT | LAYOUT_ANCHOR_BOTTOM)
		//LAYOUT_CONTROL(ID_APP_ABOUT, LAYOUT_ANCHOR_RIGHT | LAYOUT_ANCHOR_BOTTOM)
	END_LAYOUT_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(HWND, LPARAM)
	{
		// center the dialog on the screen
		CenterWindow();

		m_SystemMenu = GetSystemMenu(FALSE);
		if (m_SystemMenu.m_hMenu != NULL)
		{
			m_SystemMenu.AppendMenu(MF_SEPARATOR);
			m_SystemMenu.AppendMenu(MF_BYCOMMAND, ID_APP_ABOUT, _T("关于"));
		}

		// set icons
		HICON hIcon = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON));
		SetIcon(hIcon, TRUE);
		HICON hIconSmall = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON));
		SetIcon(hIconSmall, FALSE);

		// register object for message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->AddMessageFilter(this);
		pLoop->AddIdleHandler(this);

		UIAddChildWindowContainer(m_hWnd);

		m_edtAppExe = GetDlgItem(IDC_EDIT_APPEXE);
		m_edtAppDll = GetDlgItem(IDC_EDIT_APPDLL);
		m_btnSelectAppExe = GetDlgItem(IDC_BUTTON_SELECTAPPEXE);
		m_btnSelectAppDll = GetDlgItem(IDC_BUTTON_SELECTAPPDLL);
		m_btnSuspendProcessInject = GetDlgItem(IDC_BUTTON_SUSPENDPROCESSINJECT);
		m_btnReleaseMemorySpaces = GetDlgItem(IDC_BUTTON_RELEASEMEMORYSPACES);

		{
			this->SetWindowText(_T("注册表注入"));
			m_btnSelectAppExe.SetWindowText(_T("选择EXE文件"));
			m_btnSelectAppDll.SetWindowText(_T("选择DLL文件"));
			m_btnSuspendProcessInject.SetWindowText(_T("开启注入"));
			m_btnReleaseMemorySpaces.SetWindowText(_T("恢复注入"));
		}

		m_bGripper = TRUE;

		SetMsgHandled(FALSE);

		return TRUE;
	}

	LRESULT OnNotify(INT nID, LPNMHDR lpNMHDR)
	{
		switch (lpNMHDR->code)
		{
		case NM_RCLICK:
		{
			//if (m_ListViewCtrlData.GetNextItem(-1, LVNI_SELECTED) != -1)
			{
				CMenu menu;
				POINT point = { 0 };
				GetCursorPos(&point);
				//动态创建弹出式菜单对象
				if (menu.CreatePopupMenu())
				{
					//menu.AppendMenu(MF_STRING, IDM_SUBMENU_OPEN, _T("打开进程"));
					//menu.AppendMenu(MF_STRING, IDM_SUBMENU_ENUMOBJS, _T("枚举对象"));
					//menu.TrackPopupMenu(TPM_RIGHTBUTTON | TPM_VERPOSANIMATION | TPM_LEFTALIGN | TPM_VERTICAL, point.x, point.y, m_hWnd);
					//menu.DestroyMenu();
				}
			}
		}
		break;
		//case LVN_COLUMNCLICK:
		case HDN_ITEMCLICK:
		{
			LPNMHEADER lpNMHEADER = (LPNMHEADER)lpNMHDR;
			if (this->NColumnSortItem() != lpNMHEADER->iItem)
			{
				this->NColumnSortItem(lpNMHEADER->iItem);
				this->BColumnSortFlag(true);
			}
			else
			{
				this->BColumnSortFlag(!this->BColumnSortFlag());
			}

			int nStartPos = 0;
			int nFinalPos = 0;
			_TCHAR tValue[MAXBYTE] = { 0 };
			CString strText = this->STRColumnProperty();
			for (int i = 0;
				i < lpNMHEADER->iItem
				&& (nStartPos = strText.Find(_T("|"), nStartPos + 1));
			i++){
				;
			}
			nFinalPos = strText.Find(_T("|"), nStartPos + 1);
			lstrcpyn(tValue, (LPCTSTR)strText + nStartPos + 1, nFinalPos - nStartPos);
			this->NColumnDataType((COLUMN_DATATYPE)_ttol(tValue));
			//this->LISTVIEWCTRLWINDOW(m_ListViewCtrlData);
			//m_ListViewCtrlData.SortItems(&CompareProcess, (LPARAM)this);
		}
		break;
		case LVN_ITEMCHANGED:
		{
			//if (m_ListViewCtrlData.GetNextItem(-1, LVNI_SELECTED) != -1)
			{
				DWORD dwProcessID = 0;
				_TCHAR tText[MAXBYTE] = { 0 };
				LVITEM lvi = { 0 };
				lvi.pszText = tText;
				lvi.cchTextMax = sizeof(tText) / sizeof(_TCHAR);
				lvi.mask = LVIF_TEXT;
				lvi.iSubItem = 1;
				//if (m_ListViewCtrlData.GetSelectedItem(&lvi))
				{
					dwProcessID = _ttol(tText);
					if (dwProcessID > 0)
					{
						//OnShowmoudles(dwProcessID);
					}
				}
			}
		}
		break;
		default:
			break;
		}

		return 0;
	}

	void OnDestroy()
	{
		// unregister message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->RemoveMessageFilter(this);
		pLoop->RemoveIdleHandler(this);
	}
	void About()
	{
		CAboutDlg dlg;
		dlg.DoModal();
	}
	void OnSysCommand(UINT nID, CPoint point)
	{
		switch (nID)
		{
		case ID_APP_ABOUT:
		{
			About();
			SetMsgHandled(TRUE);
		}
		break;
		default:
		{
			SetMsgHandled(FALSE);
		}
		break;
		}
	}
	
	LRESULT OnSelectAppExe(UINT, int, HWND)
	{
		BOOL bResult = FALSE;
		_TCHAR tFileName[MAX_PATH] = { 0 };

		if (SelectOpenFile(tFileName, _T("All Files(*.*)\0*.*\0Exe Files(*.EXE)\0*.EXE\0\0")))
		{
			m_edtAppExe.SetWindowText(tFileName);
		}
		return 0;
	}

	LRESULT OnSelectAppDll(UINT, int, HWND)
	{
		BOOL bResult = FALSE;
		_TCHAR tFileName[MAX_PATH] = { 0 };

		if (SelectOpenFile(tFileName, _T("All Files(*.*)\0*.*\0Dll Files(*.DLL)\0*.DLL\0\0")))
		{
			m_edtAppDll.SetWindowText(tFileName);
		}

		return 0;
	}

	LRESULT OnSuspendProcessInject(UINT, int, HWND)
	{
		// TODO:  在此添加控件通知处理程序代码

		BOOL bRet = FALSE;
		HKEY hKey = NULL;
		LONG nReg;

		CString strAppExe(_T(""));
		CString strAppDll(_T(""));
		m_edtAppExe.GetWindowText(strAppExe);
		m_edtAppDll.GetWindowText(strAppDll);

		//打开HKEY_LOCAL_MACHINE/Software/Microsoft/WindowsNT/CurrentVersion/Windows
		nReg = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE,
			_T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Windows"),
			0,
			KEY_ALL_ACCESS,
			&hKey);

		if (nReg != ERROR_SUCCESS)
		{
			MessageBox(_T("打开注册表失败"));
			::RegCloseKey(hKey);
			return 0;
		}

		//设置AppInit_DLLs的键值为我们的Dll
		nReg = RegSetValueEx(hKey,
			_T("AppInit_DLLs"),
			0,
			REG_SZ,			
			NULL,//(byte *)m_strDllPath.GetBuffer(0)
			strAppDll.GetLength()
			);
		if (nReg != ERROR_SUCCESS)
		{
			MessageBox(_T("设置注册表失败！"));
			RegCloseKey(hKey);
			return 0;
		}

		MessageBox(_T("设置注册表成功！"));
		RegCloseKey(hKey);

		return 0;
	}

	LRESULT OnReleaseMemorySpaces(UINT, int, HWND)
	{
		BOOL bRet = FALSE;
		HKEY hKey = NULL;
		LONG nReg;

		CString strAppExe(_T(""));
		CString strAppDll(_T(""));
		m_edtAppExe.GetWindowText(strAppExe);
		m_edtAppDll.GetWindowText(strAppDll);

		//打开HKEY_LOCAL_MACHINE/Software/Microsoft/WindowsNT/CurrentVersion/Windows
		nReg = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE,
			_T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Windows"),
			0,
			KEY_ALL_ACCESS,
			&hKey);

		if (nReg != ERROR_SUCCESS)
		{
			MessageBox(_T("打开注册表失败"));
			::RegCloseKey(hKey);
			return 0;
		}

		//设置AppInit_DLLs的键值为我们的Dll
		nReg = RegSetValueEx(hKey,
			_T("AppInit_DLLs"),
			0,
			REG_SZ,
			NULL,//(byte *)m_strDllPath.GetBuffer(0)
			strAppDll.GetLength()
			);
		if (nReg != ERROR_SUCCESS)
		{
			MessageBox(_T("设置注册表失败！"));
			RegCloseKey(hKey);
			return 0;
		}

		MessageBox(_T("恢复注册表成功！"));
		RegCloseKey(hKey);

		return 0;
	}

	LRESULT OnAppAbout(UINT, int, HWND)
	{
		About();
		return 0;
	}

	LRESULT OnOK(UINT, int nID, HWND)
	{
		// TODO: Add validation code 
		CloseDialog(nID);
		return 0;
	}

	LRESULT OnCancel(UINT, int nID, HWND)
	{
		CloseDialog(nID);
		return 0;
	}

	void CloseDialog(int nValue)
	{
		EndDialog(nValue);
		//DestroyWindow();
		//::PostQuitMessage(nValue);
	}

private:

	CMenuT<false> m_SystemMenu;
	CEditT<CWindow> m_edtAppExe;
	CEditT<CWindow> m_edtAppDll;
	CButtonT<CWindow> m_btnSelectAppExe;
	CButtonT<CWindow> m_btnSelectAppDll;
	CButtonT<CWindow> m_btnSuspendProcessInject;
	CButtonT<CWindow> m_btnReleaseMemorySpaces;
};
