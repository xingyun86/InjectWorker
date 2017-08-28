// ManagerDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CodeDataBase.h"
#include <DialogLayout.h>

#include "ImportInjectDlg.h"
#include "SuspendThreadInjectDlg.h"
#include "SuspendProcessInjectDlg.h"
#include "DebuggeeInjectDlg.h"
#include "APCInjectDlg.h"
#include "RegistryInjectDlg.h"
#include "HookInjectDlg.h"
#include "RemoteThreadInjectDlg.h"
#include "IMEInjectDlg.h"
#include "HijackInjectDlg.h"


class CManagerDlg : 
	public CCodeDataBase,
	public CDialogImpl<CManagerDlg>,
	public CDialogLayout<CManagerDlg>,
	public CUpdateUI<CManagerDlg>,
	public CMessageFilter, 
	public CIdleHandler
{
public:
	enum { IDD = IDD_DIALOG_MANAGER };
	enum {
		IDM_REFRESHDISPLAY = 37000,
		IDM_IMPORTINJECT = 37001,
		IDM_SUSPENDTHREADINJECT = 37002,
		IDM_SUSPENDPROCESSINJECT = 37003,
		IDM_DEBUGGEEINJECT = 37004,
		IDM_APCINJECT = 37005,
		IDM_REGISTRYINJECT = 37006,
		IDM_HOOKINJECT = 37007,
		IDM_REMOTETHREADINJECT = 37008,
		IDM_IMEINJECT = 37009,
		IDM_HIJACKINJECT = 37010,
	};

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		return CWindow::IsDialogMessage(pMsg);
	}

	virtual BOOL OnIdle()
	{
		UIUpdateChildWindows();
		return FALSE;
	}

	BEGIN_UPDATE_UI_MAP(CManagerDlg)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CManagerDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_SYSCOMMAND(OnSysCommand)
		MSG_WM_NOTIFY(OnNotify)
		COMMAND_ID_HANDLER_EX(IDM_REFRESHDISPLAY, OnRefreshDisplay)
		COMMAND_ID_HANDLER_EX(IDM_IMPORTINJECT, OnImportInject)
		COMMAND_ID_HANDLER_EX(IDM_SUSPENDTHREADINJECT, OnSuspendThreadInject)
		COMMAND_ID_HANDLER_EX(IDM_SUSPENDPROCESSINJECT, OnSuspendProcessInject)
		COMMAND_ID_HANDLER_EX(IDM_DEBUGGEEINJECT, OnDebuggeeInject)
		COMMAND_ID_HANDLER_EX(IDM_APCINJECT, OnAPCInject)
		COMMAND_ID_HANDLER_EX(IDM_REGISTRYINJECT, OnRegistryInject)
		COMMAND_ID_HANDLER_EX(IDM_HOOKINJECT, OnHookInject)
		COMMAND_ID_HANDLER_EX(IDM_REMOTETHREADINJECT, OnRemoteThreadInject)
		COMMAND_ID_HANDLER_EX(IDM_IMEINJECT, OnIMEInject)
		COMMAND_ID_HANDLER_EX(IDM_HIJACKINJECT, OnHijackInject)
		COMMAND_ID_HANDLER_EX(ID_APP_ABOUT, OnAppAbout)
		COMMAND_ID_HANDLER_EX(IDOK, OnOK)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnCancel)
		CHAIN_MSG_MAP(CDialogLayout<CManagerDlg>)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	BEGIN_LAYOUT_MAP()
		BEGIN_LAYOUT_CONTAINER(ABS(0.0), ABS(0.0), RATIO(1.0), RATIO(0.5))
		LAYOUT_CONTROL(IDC_LIST_PROCESSES, LAYOUT_ANCHOR_ALL)
		END_LAYOUT_CONTAINER()
		BEGIN_LAYOUT_CONTAINER(ABS(0.0), RATIO(0.5), RATIO(1.0), RATIO(1.0))
		LAYOUT_CONTROL(IDC_LIST_MODULES, LAYOUT_ANCHOR_ALL)
		END_LAYOUT_CONTAINER()
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
		
		m_ListViewCtrlProcesses = GetDlgItem(IDC_LIST_PROCESSES);
		m_ListViewCtrlModules = GetDlgItem(IDC_LIST_MODULES);

		{
			this->SetWindowText(_T("注入辅助工具"));

			::SetWindowLong(m_ListViewCtrlProcesses.m_hWnd, GWL_STYLE, ::GetWindowLong(m_ListViewCtrlProcesses.m_hWnd, GWL_STYLE) | LVS_SINGLESEL);
			::SetWindowLong(m_ListViewCtrlProcesses.m_hWnd, GWL_EXSTYLE, ::GetWindowLong(m_ListViewCtrlProcesses.m_hWnd, GWL_EXSTYLE) | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
			m_ListViewCtrlProcesses.SetExtendedListViewStyle(m_ListViewCtrlProcesses.GetExtendedListViewStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
			::SetWindowLong(m_ListViewCtrlModules.m_hWnd, GWL_STYLE, ::GetWindowLong(m_ListViewCtrlModules.m_hWnd, GWL_STYLE) | LVS_SINGLESEL);
			::SetWindowLong(m_ListViewCtrlModules.m_hWnd, GWL_EXSTYLE, ::GetWindowLong(m_ListViewCtrlModules.m_hWnd, GWL_EXSTYLE) | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
			m_ListViewCtrlModules.SetExtendedListViewStyle(m_ListViewCtrlModules.GetExtendedListViewStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

			m_ListViewCtrlProcesses.InsertColumn(0, _T("进程名称"), LVCFMT_LEFT, 150, -1);
			m_ListViewCtrlProcesses.InsertColumn(1, _T("进程ID"), LVCFMT_LEFT, 150, -1);
			m_ListViewCtrlProcesses.InsertColumn(2, _T("映像路径"), LVCFMT_LEFT, 500, -1);

			m_ListViewCtrlModules.InsertColumn(0, _T("模块名称"), LVCFMT_LEFT, 150, -1);
			m_ListViewCtrlModules.InsertColumn(1, _T("模块路径"), LVCFMT_LEFT, 500, -1);
		
			OnShowprocess();
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
			if (::GetDlgCtrlID(lpNMHDR->hwndFrom) == IDC_LIST_PROCESSES && m_ListViewCtrlProcesses.GetNextItem(-1, LVNI_SELECTED) != -1)
			{
				CMenu menu;
				POINT point = { 0 };
				GetCursorPos(&point);
				//动态创建弹出式菜单对象
				if (menu.CreatePopupMenu())
				{
					menu.AppendMenu(MF_STRING, IDM_REFRESHDISPLAY, _T("刷新显示"));
					menu.AppendMenu(MF_STRING, IDM_IMPORTINJECT, _T("导入表注入"));
					menu.AppendMenu(MF_STRING, IDM_SUSPENDTHREADINJECT, _T("挂起线程注入"));
					menu.AppendMenu(MF_STRING, IDM_SUSPENDPROCESSINJECT, _T("挂起进程注入"));
					menu.AppendMenu(MF_STRING, IDM_DEBUGGEEINJECT, _T("调试器注入"));
					menu.AppendMenu(MF_STRING, IDM_APCINJECT, _T("APC注入"));
					menu.AppendMenu(MF_STRING, IDM_REGISTRYINJECT, _T("注册表注入"));
					menu.AppendMenu(MF_STRING, IDM_HOOKINJECT, _T("钩子注入"));
					menu.AppendMenu(MF_STRING, IDM_REMOTETHREADINJECT, _T("远程线程注入"));
					menu.AppendMenu(MF_STRING, IDM_IMEINJECT, _T("输入法注入"));
					menu.AppendMenu(MF_STRING, IDM_HIJACKINJECT, _T("DLL劫持注入"));
					menu.TrackPopupMenu(TPM_RIGHTBUTTON | TPM_VERPOSANIMATION | TPM_LEFTALIGN | TPM_VERTICAL, point.x, point.y, m_hWnd);
					menu.DestroyMenu();
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
			
			if (m_ListViewCtrlProcesses.GetHeader().m_hWnd == lpNMHDR->hwndFrom)
			{
				CString strText(_T(""));
				m_ListViewCtrlProcesses.GetHeader().GetWindowText(strText);
				for (int i = 0;
					i < lpNMHEADER->iItem
					&& (nStartPos = strText.Find(_T("|"), nStartPos + 1));
				i++){
					;
				}
				nFinalPos = strText.Find(_T("|"), nStartPos + 1);
				lstrcpyn(tValue, (LPCTSTR)strText + nStartPos + 1, nFinalPos - nStartPos);
				this->NColumnDataType((COLUMN_DATATYPE)_ttol(tValue));

				this->LISTVIEWCTRLWINDOW(m_ListViewCtrlProcesses);
				m_ListViewCtrlProcesses.SortItems(&CompareProcess, (LPARAM)this);
			}
			else if (m_ListViewCtrlModules.GetHeader().m_hWnd == lpNMHDR->hwndFrom)
			{
				CString strText(_T(""));
				m_ListViewCtrlModules.GetHeader().GetWindowText(strText);
				for (int i = 0;
					i < lpNMHEADER->iItem
					&& (nStartPos = strText.Find(_T("|"), nStartPos + 1));
				i++){
					;
				}
				nFinalPos = strText.Find(_T("|"), nStartPos + 1);
				lstrcpyn(tValue, (LPCTSTR)strText + nStartPos + 1, nFinalPos - nStartPos);
				this->NColumnDataType((COLUMN_DATATYPE)_ttol(tValue));

				this->LISTVIEWCTRLWINDOW(m_ListViewCtrlModules);
				m_ListViewCtrlModules.SortItems(&CompareProcess, (LPARAM)this);
			}			
		}
		break;
		case LVN_ITEMCHANGED:
		{
			switch (::GetDlgCtrlID(lpNMHDR->hwndFrom))
			{
			case IDC_LIST_PROCESSES:
				{
					if (m_ListViewCtrlProcesses.GetNextItem(-1, LVNI_SELECTED) != -1)
					{
						DWORD dwProcessID = 0;
						_TCHAR tText[MAXBYTE] = { 0 };
						LVITEM lvi = { 0 };
						lvi.pszText = tText;
						lvi.cchTextMax = sizeof(tText) / sizeof(_TCHAR);
						lvi.mask = LVIF_TEXT;
						lvi.iSubItem = 1;
						if (m_ListViewCtrlProcesses.GetSelectedItem(&lvi))
						{
							dwProcessID = _ttol(tText);
							if (dwProcessID > 0)
							{
								OnShowmoudles(dwProcessID);
							}
						}
					}
				}
				break;
			default:
				break;
			}
		}
		break;
		default:
			break;
		}

		return 0;
	}

	/*******************************************************
	*函数功能:遍历所有的进程并且显示
	*函数参数:无
	*函数返回:无
	*注意事项:无
	*最后修改时间:2017/5/13
	*******************************************************/
	void OnShowprocess()
	{
		m_ListViewCtrlProcesses.DeleteAllItems();
		PROCESSENTRY32 pe32 = { 0 };
		pe32.dwSize = sizeof(PROCESSENTRY32);

		HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

		if (hProcessSnap == INVALID_HANDLE_VALUE)
		{
			MessageBox(_T("获取系统进程快照失败！！！"));
			return;
		}

		int nRowIdx = 0;
		int nColIdx = 0;
		BOOL bResult = Process32First(hProcessSnap, &pe32);
		
		this->NListViewCtrlType(LVCT_PROCESSES);
		this->STRColumnProperty(_T("|3|1|3"));
		m_ListViewCtrlProcesses.GetHeader().SetWindowText(_T("|3|1|3"));
		while (bResult)
		{
			CString myStr = pe32.szExeFile;
			CString myStrId(_T(""));
			nColIdx = 0;
		
			myStrId.Format(_T("%lu"), pe32.th32ProcessID);
			m_ListViewCtrlProcesses.InsertItem(nRowIdx, myStr);
			m_ListViewCtrlProcesses.SetItemText(nRowIdx, ++nColIdx, myStrId);
			m_ListViewCtrlProcesses.SetItemText(nRowIdx, ++nColIdx, pe32.szExeFile);
			
			//设置每项的ItemData为数组中数据的索引
			//在排序函数中通过该ItemData来确定数据
			m_ListViewCtrlProcesses.SetItemData(nRowIdx, nRowIdx);
			
			nRowIdx++;
			
			bResult = Process32Next(hProcessSnap, &pe32);
		}

		CloseHandle(hProcessSnap);
	}

	/*******************************************************
	*函数功能:按照进程ID显示所有DLL模块
	*函数参数:参数1：当前列表行数
	*函数返回:无
	*注意事项:无
	*最后修改时间:2017/5/13
	*******************************************************/
	void OnShowmoudles(DWORD dwProcessID)
	{
		m_ListViewCtrlModules.DeleteAllItems();

		EnableDebugPriv();

		HANDLE hModuleSnap = INVALID_HANDLE_VALUE;

		this->NListViewCtrlType(LVCT_MODULES);
		this->STRColumnProperty(_T("|3|3"));
		m_ListViewCtrlModules.GetHeader().SetWindowText(_T("|3|3"));

		hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwProcessID);
		if (hModuleSnap == INVALID_HANDLE_VALUE){
			printf("CreateToolhelp32SnapshotError! \n");
			return;
		}

		int nColIdx = 0;
		int nRowIdx = 0;
		MODULEENTRY32 module32 = { 0 };
		module32.dwSize = sizeof(module32);
		BOOL bResult = Module32First(hModuleSnap, &module32);
		while (bResult)
		{
			CString strDllName = module32.szModule;
			CString strDllPath = module32.szExePath;
			nColIdx = 0;

			m_ListViewCtrlModules.InsertItem(nRowIdx, strDllName);
			m_ListViewCtrlModules.SetItemText(nRowIdx, ++nColIdx, strDllPath);
			
			//设置每项的ItemData为数组中数据的索引
			//在排序函数中通过该ItemData来确定数据
			m_ListViewCtrlModules.SetItemData(nRowIdx, nRowIdx);

			nRowIdx++;

			bResult = Module32Next(hModuleSnap, &module32);
		}

		CloseHandle(hModuleSnap);
	}
	void OnDestroy()
	{
		// unregister message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->RemoveMessageFilter(this);
		pLoop->RemoveIdleHandler(this);
	}

	LRESULT OnRefreshDisplay(UINT, int, HWND)
	{
		OnShowprocess();
		return 0;
	}
	LRESULT OnImportInject(UINT, int, HWND)
	{
		CImportInjectDlg dlg;
		dlg.DoModal();
		return 0;
	}
	LRESULT OnSuspendThreadInject(UINT, int, HWND)
	{
		CSuspendThreadInjectDlg dlg;
		dlg.DoModal();
		return 0;
	}
	LRESULT OnSuspendProcessInject(UINT, int, HWND)
	{
		CSuspendProcessInjectDlg dlg;
		dlg.DoModal();
		return 0;
	}
	LRESULT OnDebuggeeInject(UINT, int, HWND)
	{
		CDebuggeeInjectDlg dlg;
		dlg.DoModal();
		return 0;
	}
	LRESULT OnAPCInject(UINT, int, HWND)
	{
		CAPCInjectDlg dlg;
		dlg.DoModal();
		return 0;
	}
	LRESULT OnRegistryInject(UINT, int, HWND)
	{
		CRegistryInjectDlg dlg;
		dlg.DoModal();
		return 0;
	}
	LRESULT OnHookInject(UINT, int, HWND)
	{
		CHookInjectDlg dlg;
		dlg.DoModal();
		return 0;
	}
	LRESULT OnRemoteThreadInject(UINT, int, HWND)
	{
		CRemoteThreadInjectDlg dlg;
		dlg.DoModal();
		return 0;
	}
	LRESULT OnIMEInject(UINT, int, HWND)
	{
		CIMEInjectDlg dlg;
		dlg.DoModal();
		return 0;
	}
	LRESULT OnHijackInject(UINT, int, HWND)
	{
		CHijackInjectDlg dlg;
		dlg.DoModal();
		return 0;
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
		DestroyWindow();
		::PostQuitMessage(nValue);
	}

private:

	CMenuT<false> m_SystemMenu;
	CListViewCtrlT<CWindow> m_ListViewCtrlProcesses;
	CListViewCtrlT<CWindow> m_ListViewCtrlModules;
};
