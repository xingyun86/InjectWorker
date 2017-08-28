// ManagerDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <sys/stat.h>

class CCodeDataBase
{
public:
	typedef enum LISTVIEWCTRL_TYPE {
		LVCT_NULL = 0,
		LVCT_PROCESSES = 1,
		LVCT_MODULES = 2,
		LVCT_OTHERS
	};
	typedef enum COLUMN_DATATYPE{
		CDT_NULL = 0,
		CDT_DEC = 1,
		CDT_HEX = 2,
		CDT_STRING = 3,
		CDT_OTHERS,
	};

	//程序实例只允许一个
	__inline static BOOL RunAppOnce(tstring tsName)
	{
		HANDLE hMutexInstance = ::CreateMutex(NULL, FALSE, tsName.c_str());  //创建互斥
		if (hMutexInstance)
		{
			if (::GetLastError() == ERROR_ALREADY_EXISTS)
			{
				//OutputDebugString(_T("互斥检测返回！"));
				::CloseHandle(hMutexInstance);
				return FALSE;
			}
		}
		return TRUE;
	}
	__inline static void RegisterDropFilesEvent(HWND hWnd)
	{
#ifndef WM_COPYGLOBALDATA
#define WM_COPYGLOBALDAYA	0x0049
#endif

#ifndef MSGFLT_ADD
#define MSGFLT_ADD 1
#endif

#ifndef MSGFLT_REMOVE
#define MSGFLT_REMOVE 2
#endif
		SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_ACCEPTFILES);
		typedef BOOL (WINAPI *LPFN_ChangeWindowMessageFilter)(__in UINT message, __in DWORD dwFlag);
		LPFN_ChangeWindowMessageFilter pfnChangeWindowMessageFilter = (LPFN_ChangeWindowMessageFilter)GetProcAddress(GetModuleHandle(_T("USER32.DLL")), "ChangeWindowMessageFilter");
		if (pfnChangeWindowMessageFilter)
		{
			pfnChangeWindowMessageFilter(WM_DROPFILES, MSGFLT_ADD);
			pfnChangeWindowMessageFilter(WM_COPYDATA, MSGFLT_ADD);
			pfnChangeWindowMessageFilter(WM_COPYGLOBALDAYA, MSGFLT_ADD);// 0x0049 == WM_COPYGLOBALDATA
		}
	}
	__inline static size_t GetDropFiles(std::map<TSTRING, TSTRING> & ttmap, HDROP hDropInfo)
	{
		UINT nIndex = 0;
		UINT nNumOfFiles = 0;
		_TCHAR tszFilePathName[MAX_PATH + 1] = { 0 };

		//得到文件个数
		nNumOfFiles = DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 0);

		for (nIndex = 0; nIndex < nNumOfFiles; nIndex++)
		{
			//得到文件名
			DragQueryFile(hDropInfo, nIndex, (LPTSTR)tszFilePathName, _MAX_PATH);
			ttmap.insert(std::map<TSTRING, TSTRING>::value_type(tszFilePathName, tszFilePathName));
		}

		DragFinish(hDropInfo);

		return nNumOfFiles;
	}
	//获取程序工作路径
	__inline static tstring GetWorkPath()
	{
		tstring tsWorkPath = _T("");
		_TCHAR tWorkPath[MAX_PATH] = { 0 };
		::GetCurrentDirectory(MAX_PATH, tWorkPath);
		if (*tWorkPath)
		{
			tsWorkPath = tstring(tWorkPath) + _T("\\");
		}
		return tsWorkPath;
	}

	//获取系统临时路径
	__inline static tstring GetTempPath()
	{
		_TCHAR tTempPath[MAX_PATH] = { 0 };
		::GetTempPath(MAX_PATH, tTempPath);
		return tstring(tTempPath);
	}

	//获取程序文件路径
	__inline static tstring GetProgramPath()
	{
		tstring tsFilePath = _T("");
		_TCHAR * pFoundPosition = 0;
		_TCHAR tFilePath[MAX_PATH] = { 0 };
		::GetModuleFileName(NULL, tFilePath, MAX_PATH);
		if (*tFilePath)
		{
			pFoundPosition = _tcsrchr(tFilePath, _T('\\'));
			if (*(++pFoundPosition))
			{
				*pFoundPosition = _T('\0');
			}
			tsFilePath = tFilePath;
		}
		return tsFilePath;
	}

	//获取系统路径
	__inline static tstring GetSystemPath()
	{
		tstring tsSystemPath = _T("");
		_TCHAR tSystemPath[MAX_PATH] = { 0 };
		::GetSystemDirectory(tSystemPath, MAX_PATH);
		if (*tSystemPath)
		{
			tsSystemPath = tstring(tSystemPath) + _T("\\");
		}
		return tsSystemPath;
	}

	//判断目录是否存在
	__inline static BOOL IsDirectoryExists(LPCTSTR lpDirectory)
	{
		BOOL bResult = TRUE;
		struct _stat st = { 0 };
		if ((_tstat(lpDirectory, &st) != 0) || (st.st_mode & S_IFDIR != S_IFDIR))
		{
			bResult = FALSE;
		}

		return bResult;
	}
	//判断目录是否存在，若不存在则创建
	__inline static BOOL CreateCascadeDirectory(LPCTSTR lpPathName,        //Directory name
		LPSECURITY_ATTRIBUTES lpSecurityAttributes/* = NULL*/  // Security attribute
		)
	{
		if (IsDirectoryExists(lpPathName))       //如果目录已存在，直接返回
		{
			return TRUE;
		}

		_TCHAR tPathSect[MAX_PATH] = { 0 };
		_TCHAR tPathName[MAX_PATH] = { 0 };
		_tcscpy(tPathName, lpPathName);
		_TCHAR *pToken = _tcstok(tPathName, _T("\\"));
		while (pToken)
		{
			_sntprintf(tPathSect, sizeof(tPathSect) / sizeof(_TCHAR), _T("%s%s\\"), tPathSect, pToken);
			if (!IsDirectoryExists(tPathSect))
			{
				//创建失败时还应删除已创建的上层目录，此次略
				if (!::CreateDirectory(tPathSect, lpSecurityAttributes))
				{
					_tprintf(_T("CreateDirectory Failed: %d\n"), GetLastError());
					return FALSE;
				}
			}
			pToken = _tcstok(NULL, _T("\\"));
		}
		return TRUE;
	}
#define CMD_PATH_NAME				"cmd.exe" //相对路径名称

	//获取cmd.exe文件路径
	__inline static tstring GetCmdPath()
	{
		return GetSystemPath() + _T(CMD_PATH_NAME);
	}
	
	BOOL SelectSaveFile(_TCHAR(&tFileName)[MAX_PATH], const _TCHAR * ptFilter = _T("Audio Files (*.WAV)\0*.WAV\0All Files (*.*)\0*.*\0\0"))
	{
		BOOL bResult = FALSE;
		OPENFILENAME ofn = { 0 };
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.lpstrFilter = ptFilter;
		ofn.lpstrFile = tFileName;
		ofn.nMaxFile = MAX_PATH;
		ofn.Flags = OFN_EXPLORER | OFN_ENABLEHOOK | OFN_HIDEREADONLY | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST;
		bResult = GetSaveFileName(&ofn);
		if (bResult == FALSE)
		{
			//dwError = CommDlgExtendedError();
			//return bResult;
		}
		return bResult;
	}
	BOOL SelectOpenFile(_TCHAR(&tFileName)[MAX_PATH], const _TCHAR * ptFilter = _T("All Files(*.*)\0*.*\0Execute Files(*.EXE)\0*.EXE\0Dll Files(*.DLL)\0*.DLL\0Dll Files(*.SYS)\0*.SYS\0\0"))
	{
		BOOL bResult = FALSE;
		OPENFILENAME ofn = { 0 };
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.lpstrFilter = ptFilter;
		ofn.lpstrFile = tFileName;
		ofn.nMaxFile = MAX_PATH;
		ofn.Flags = OFN_EXPLORER | OFN_ENABLEHOOK | OFN_HIDEREADONLY | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST;
		bResult = GetOpenFileName(&ofn);
		if (bResult == FALSE)
		{
			//dwError = CommDlgExtendedError();
			//return bResult;
		}
		return bResult;
	}
	typedef struct
	{
		HWND hWnd;
		DWORD dwPid;
	}WNDINFO;

	static BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam)
	{
		WNDINFO* pInfo = (WNDINFO*)lParam;
		DWORD dwProcessId = 0;
		GetWindowThreadProcessId(hWnd, &dwProcessId);

		if (dwProcessId == pInfo->dwPid)
		{
			pInfo->hWnd = hWnd;
			return FALSE;
		}
		return TRUE;
	}

	/*******************************************************
	*函数功能:按照进程ID获取主窗口句柄
	*函数参数:参数1：进程ID
	*函数返回:HWND
	*注意事项:无
	*最后修改时间:2017/5/13
	*******************************************************/
	HWND GetHwndByProcessId(DWORD dwProcessId)
	{
		WNDINFO info = { 0 };
		info.hWnd = NULL;
		info.dwPid = dwProcessId;
		EnumWindows(EnumWindowsProc, (LPARAM)&info);
		return info.hWnd;
	}
	/*******************************************************
	*函数功能:提权代码
	*函数参数:无
	*函数返回:bool
	*注意事项:无
	*最后修改时间:2017/5/13
	*******************************************************/
	bool EnableDebugPriv() //提权代码
	{
		HANDLE hToken;

		LUID sedebugnameValue;

		TOKEN_PRIVILEGES tkp;

		if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
		{
			return false;
		}
		if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &sedebugnameValue))
		{
			CloseHandle(hToken);

			return false;
		}

		tkp.PrivilegeCount = 1;

		tkp.Privileges[0].Luid = sedebugnameValue;

		tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

		if (!AdjustTokenPrivileges(hToken, FALSE, &tkp, sizeof(tkp), NULL, NULL))
		{
			CloseHandle(hToken);

			return false;
		}
		return true;
	}
	// Sort the item in reverse alphabetical order.
	static int CALLBACK CompareProcess(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
	{
		// lParamSort contains a pointer to the list view control.
		int nResult = 0;
		unsigned long ulX = 0;
		unsigned long ulY = 0;
		COLUMN_DATATYPE cdt = CDT_NULL;
		bool bColumnSortFlag = false;
		int nColumnSortItem = 0;
		LVFINDINFO info = { 0 };
		int nCompareSortIndex1 = 0;
		int nCompareSortIndex2 = 0;

		CString    strItem1(_T(""));
		CString    strItem2(_T(""));
		CListViewCtrlT<CWindow> listViewCtrl;
		CCodeDataBase * pCDB = (CCodeDataBase *)lParamSort;
		if (pCDB)
		{
			cdt = pCDB->NColumnDataType();
			bColumnSortFlag = pCDB->BColumnSortFlag();
			nColumnSortItem = pCDB->NColumnSortItem();
			listViewCtrl = pCDB->LISTVIEWCTRLWINDOW();

			info.flags = LVFI_PARAM;
			info.lParam = lParam1;
			nCompareSortIndex1 = listViewCtrl.FindItem(&info, -1);
			info.lParam = lParam2;
			nCompareSortIndex2 = listViewCtrl.FindItem(&info, -1);

			listViewCtrl.GetItemText(nCompareSortIndex1, nColumnSortItem, strItem1);
			listViewCtrl.GetItemText(nCompareSortIndex2, nColumnSortItem, strItem2);
		}

		switch (cdt)
		{
		case CDT_NULL:
			break;
		case CDT_DEC:
			ulX = _tcstoul(strItem2, 0, 10);
			ulY = _tcstoul(strItem1, 0, 10);
			nResult = ulX - ulY;
			break;
		case CDT_HEX:
			ulX = _tcstoul(strItem2, 0, 16);
			ulY = _tcstoul(strItem1, 0, 16);
			nResult = ulX - ulY;
			break;
		case CDT_STRING:
			nResult = strItem2.CompareNoCase(strItem1);
			break;
		case CDT_OTHERS:
			break;
		default:
			break;
		}

		return (bColumnSortFlag ? (nResult) : (-nResult));
	}

	virtual CListViewCtrlT<CWindow> LISTVIEWCTRLWINDOW()
	{
		return this->m_ListViewCtrlWindow;
	}
	void LISTVIEWCTRLWINDOW(CListViewCtrlT<CWindow> & m_ListViewCtrlWindow)
	{
		this->m_ListViewCtrlWindow = m_ListViewCtrlWindow;
	}
	void NColumnSortItem(int nColumnSortItem)
	{
		this->m_nColumnSortItem = nColumnSortItem;
	}
	int NColumnSortItem()
	{
		return this->m_nColumnSortItem;
	}
	void BColumnSortFlag(int nColumnSortFlag)
	{
		this->m_nColumnSortFlag = nColumnSortFlag;
	}
	int BColumnSortFlag()
	{
		return this->m_nColumnSortFlag;
	}

	void NListViewCtrlType(LISTVIEWCTRL_TYPE nListViewCtrlType)
	{
		this->m_nListViewCtrlType = nListViewCtrlType;
	}
	LISTVIEWCTRL_TYPE NListViewCtrlType()
	{
		return this->m_nListViewCtrlType;
	}
	void NColumnDataType(COLUMN_DATATYPE nColumnDataType)
	{
		this->m_nColumnDataType = nColumnDataType;
	}
	COLUMN_DATATYPE NColumnDataType()
	{
		return this->m_nColumnDataType;
	}
	void STRColumnProperty(CString strColumnProperty)
	{
		this->m_strColumnProperty = strColumnProperty;
	}
	CString STRColumnProperty()
	{
		return this->m_strColumnProperty;
	}
private:
	LISTVIEWCTRL_TYPE m_nListViewCtrlType;
	CString m_strColumnProperty;
	COLUMN_DATATYPE m_nColumnDataType;
	bool m_nColumnSortFlag;
	int  m_nColumnSortItem;

	CListViewCtrlT<CWindow> m_ListViewCtrlWindow;
};