// ImportInjectDlg.h : interface of the CImportInjectDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CodeDataBase.h"
#include <DialogLayout.h>
#include "PEFuncs.h"

#define TMP_EXE "tmp.exe"

class CImportInjectDlg :
	public CCodeDataBase,
	public CDialogImpl<CImportInjectDlg>,
	public CDialogLayout<CImportInjectDlg>,
	public CUpdateUI<CImportInjectDlg>,
	public CMessageFilter,
	public CIdleHandler
{
public:
	enum { IDD = IDD_DIALOG_IMPORTINJECT };

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		return CWindow::IsDialogMessage(pMsg);
	}

	virtual BOOL OnIdle()
	{
		UIUpdateChildWindows();
		return FALSE;
	}

	BEGIN_UPDATE_UI_MAP(CImportInjectDlg)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CImportInjectDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_SYSCOMMAND(OnSysCommand)
		MSG_WM_NOTIFY(OnNotify)
		COMMAND_ID_HANDLER_EX(IDC_BUTTON_SELECTAPPEXE, OnSelectAppExe)
		COMMAND_ID_HANDLER_EX(IDC_BUTTON_ADDAPPFUN, OnAddAppFun)
		COMMAND_ID_HANDLER_EX(IDC_BUTTON_IMPORTINJECT, OnImportInject)
		COMMAND_ID_HANDLER_EX(ID_APP_ABOUT, OnAppAbout)
		COMMAND_ID_HANDLER_EX(IDOK, OnOK)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnCancel)
		CHAIN_MSG_MAP(CDialogLayout<CImportInjectDlg>)
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
		memset(&m_MapFileStruct, 0, sizeof(m_MapFileStruct));

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
		m_edtAppFun = GetDlgItem(IDC_EDIT_APPFUN);
		m_btnSelectAppExe = GetDlgItem(IDC_BUTTON_SELECTAPPEXE);
		m_btnAddAppFun = GetDlgItem(IDC_BUTTON_ADDAPPFUN);
		m_btnImportInject = GetDlgItem(IDC_BUTTON_IMPORTINJECT);
		m_ListViewCtrlData = GetDlgItem(IDC_LIST_DATA);

		{
			this->SetWindowText(_T("导入表注入"));
			m_btnSelectAppExe.SetWindowText(_T("选择文件"));
			m_btnAddAppFun.SetWindowText(_T("添加函数"));
			m_btnImportInject.SetWindowText(_T("导入表注入"));

			::SetWindowLong(m_ListViewCtrlData.m_hWnd, GWL_STYLE, ::GetWindowLong(m_ListViewCtrlData.m_hWnd, GWL_STYLE) | LVS_SINGLESEL);
			::SetWindowLong(m_ListViewCtrlData.m_hWnd, GWL_EXSTYLE, ::GetWindowLong(m_ListViewCtrlData.m_hWnd, GWL_EXSTYLE) | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
			m_ListViewCtrlData.SetExtendedListViewStyle(m_ListViewCtrlData.GetExtendedListViewStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
			m_ListViewCtrlData.InsertColumn(0, _T("函数名称"), LVCFMT_LEFT, 150, -1);
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
			if (m_ListViewCtrlData.GetNextItem(-1, LVNI_SELECTED) != -1)
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
			this->LISTVIEWCTRLWINDOW(m_ListViewCtrlData);
			m_ListViewCtrlData.SortItems(&CompareProcess, (LPARAM)this);
		}
		break;
		case LVN_ITEMCHANGED:
		{
			if (m_ListViewCtrlData.GetNextItem(-1, LVNI_SELECTED) != -1)
			{
				DWORD dwProcessID = 0;
				_TCHAR tText[MAXBYTE] = { 0 };
				LVITEM lvi = { 0 };
				lvi.pszText = tText;
				lvi.cchTextMax = sizeof(tText) / sizeof(_TCHAR);
				lvi.mask = LVIF_TEXT;
				lvi.iSubItem = 1;
				if (m_ListViewCtrlData.GetSelectedItem(&lvi))
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

		if (SelectOpenFile(tFileName, _T("All Files(*.*)\0*.*\0Execute Files(*.EXE)\0*.EXE\0\0")))
		{
			m_edtAppExe.SetWindowText(tFileName);

			//复制一份文件用于修改，源文件保留。
			bResult = ::CopyFile(tFileName, tstring(CCodeDataBase::GetTempPath() + _T(TMP_EXE)).c_str(), FALSE);
			if (bResult)
			{
				LoadFileR(tFileName, &m_MapFileStruct);
				
				if (!IsPEFile(m_MapFileStruct.ImageBase))
				{
					MessageBox(_T("不是有效的PE文件"), _T("不是有效的PE文件"), MB_OK);
					UnLoadFile(&m_MapFileStruct);
					return 0;
				}
			}
		}

		return 0;
	}

	LRESULT OnAddAppFun(UINT, int, HWND)
	{
		LVITEM lvi = { 0 };
		CString strAppFun(_T(""));

		m_edtAppFun.GetWindowText(strAppFun);
		if (strAppFun.GetLength() <= 0)
		{
			MessageBox(_T("请输入DLL函数名"));
			return 0;
		}
		lvi.iItem = m_ListViewCtrlData.GetItemCount();
		lvi.iSubItem = 0;
		lvi.mask = LVIF_TEXT;
		lvi.pszText = (LPTSTR)(LPCTSTR)strAppFun;
		m_ListViewCtrlData.InsertItem(&lvi);

		return 0;
	}

	/*******************************************************
	*函数功能:计算内存对齐或者文件对齐后的大小
	*函数参数:参数1：实际大小，参数2：对齐值
	*函数返回:DWORD
	*注意事项:无
	*最后修改时间:2017/5/13
	*******************************************************/
	DWORD ClacAlignment(DWORD dwSize, DWORD dwAlign)
	{
		if (dwSize % dwAlign != 0)
		{
			return (dwSize / dwAlign + 1)*dwAlign;
		}
		else
		{
			return dwSize;
		}
	}

	LRESULT OnImportInject(UINT, int, HWND)
	{
		FILE* fp;
		//最后一个节
		PIMAGE_SECTION_HEADER lpImgLastSection;
		//要添加的区块
		IMAGE_SECTION_HEADER ImgNewSection;
		//第一个节头
		PIMAGE_SECTION_HEADER lpFirstSectionHeader;
		//打开源文件修改。
		PIMAGE_NT_HEADERS lpNtHeader = new IMAGE_NT_HEADERS;
		PIMAGE_NT_HEADERS lpNewNtHeader = new IMAGE_NT_HEADERS;
		//节的数目
		int nSectionNum = 0;
		//新节的RVA
		DWORD dwNewSectionRVA, dwNewImportRva;
		//新节的文件偏移
		DWORD dwNewFA = 0;
		//节对齐
		int nSectionAlignment = 0;
		//文件对齐
		int nFileAlignment = 0;
		//DLL名称的长度
		int nDllLen = 0;
		//需要写入的函数数目
		int nFunNum = m_ListViewCtrlData.GetItemCount();
		//相对于新节的文件偏移
		DWORD dwNewOffset = 0;
		//要添加的节表头
		//IMAGE_SECTION_HEADER ImgNewSection;
		PIMAGE_IMPORT_DESCRIPTOR lpImport, lpNewImport;
		//原来导入表的大小，和新导入表的大小
		DWORD dwImportSize, dwNewImportSize;
		//计算新节头的文件偏移
		DWORD dwNewSectionOffset;
		int i = 0;
		CString strTmpExe(_T(""));
		CString strAppDll(_T(""));

		strTmpExe = tstring(CCodeDataBase::GetTempPath() + _T(TMP_EXE)).c_str();
		m_edtAppDll.GetWindowText(strAppDll);
		if (strAppDll.GetLength() <= 0)
		{
			MessageBox(_T("请输入DLL函数名"));
			return 0;
		}

		fp = ::fopen(CStringA(strTmpExe), ("rb+"));
		if (fp == NULL)
		{
			::DeleteFile(strTmpExe);
			MessageBox(_T("打开临时文件失败！！"));
			return 0;
		}

		lpFirstSectionHeader = GetFirstSectionHeader(m_MapFileStruct.ImageBase);
		lpNtHeader = GetNtHeaders(m_MapFileStruct.ImageBase);
		nSectionNum = lpNtHeader->FileHeader.NumberOfSections;
		nSectionAlignment = lpNtHeader->OptionalHeader.SectionAlignment;
		nFileAlignment = lpNtHeader->OptionalHeader.FileAlignment;

		//获取导入表的指针
		lpImport = (PIMAGE_IMPORT_DESCRIPTOR)ImageDirectoryEntryToData(m_MapFileStruct.ImageBase, FALSE, IMAGE_DIRECTORY_ENTRY_IMPORT, &dwImportSize);
		//计算新的导入表的大小:旧的导入表大小 + 新的导入表大小
		dwNewImportSize = dwImportSize + sizeof(IMAGE_IMPORT_DESCRIPTOR);
		//获取最后一个节头
		lpImgLastSection = lpFirstSectionHeader + (nSectionNum - 1);
		//获取新节的RVA
		dwNewSectionRVA = lpImgLastSection->VirtualAddress
			+ ClacAlignment(lpImgLastSection->Misc.VirtualSize, nSectionAlignment);
		//计算新的文件偏移
		dwNewFA = lpImgLastSection->PointerToRawData
			+ ClacAlignment(lpImgLastSection->SizeOfRawData, nFileAlignment);

		//1.在复制的文件中写入DLL名
		fseek(fp, dwNewFA, SEEK_SET);
		dwNewOffset = CStringA(strAppDll).GetLength() + 1;
		fwrite(CStringA(strAppDll), dwNewOffset, 1, fp);

		DWORD *arrINTRva = new DWORD[nFunNum + 1];
		memset(arrINTRva, 0, sizeof(DWORD)*(nFunNum + 1));

		//2.写入所有的的IMAGE_IMPORT_BY_NAME结构,也就是写入所有函数名
		int nFunLen = 0;
		PIMAGE_IMPORT_BY_NAME pIIBN = new IMAGE_IMPORT_BY_NAME;
		DWORD dwTempRva = 0;
		CString strFunName(_T(""));
		for (i = 0; i < nFunNum; i++)
		{
			nFunLen = 0;
			dwTempRva = 0;
			strFunName = _T("");

			pIIBN->Hint = i;
			
			m_ListViewCtrlData.GetItemText(i, 0, strFunName);
			fseek(fp, dwNewFA + dwNewOffset, SEEK_SET);
			//计算IMAGE_IMPORT_BY_NAME的RVA存入数组
			dwTempRva = dwNewSectionRVA + dwNewOffset;
			arrINTRva[i] = dwTempRva;
			dwNewOffset = dwNewOffset + CStringA(strFunName).GetLength() + 1 + sizeof(WORD);
			memcpy(pIIBN->Name, CStringA(strFunName), CStringA(strFunName).GetLength() + 1);
			fwrite(pIIBN, CStringA(strFunName).GetLength() + 1 + sizeof(WORD), 1, fp);
			memset(pIIBN->Name, 0, CStringA(strFunName).GetLength() + 1);
		}

		DWORD dwINTRVA = dwNewSectionRVA + dwNewOffset;

		//3.写入所有的的INT结构
		for (i = 0; i < nFunNum + 1; i++)
		{
			fseek(fp, dwNewFA + dwNewOffset, SEEK_SET);
			dwNewOffset += sizeof(DWORD);
			//末尾填充0结构体
			fwrite(&arrINTRva[i], sizeof(DWORD), 1, fp);
		}

		//4.申请新空间存放旧的的IID和新的IID
		lpNewImport = (PIMAGE_IMPORT_DESCRIPTOR)malloc(dwNewImportSize);
		memset(lpNewImport, 0, dwNewImportSize);
		memcpy(lpNewImport, lpImport, dwImportSize);

		i = 0;
		while (1)
		{
			if (lpNewImport[i].OriginalFirstThunk == 0 && lpNewImport[i].TimeDateStamp == 0 &&
				lpNewImport[i].ForwarderChain == 0 && lpNewImport[i].Name == 0 && lpNewImport[i].FirstThunk == 0)
			{
				lpNewImport[i].Name = dwNewSectionRVA;
				lpNewImport[i].TimeDateStamp = 0;
				lpNewImport[i].ForwarderChain = 0;
				lpNewImport[i].FirstThunk = dwINTRVA;
				lpNewImport[i].OriginalFirstThunk = dwINTRVA;
				break;
			}
			else
			{
				i++;
			}
		}
		//计算新的导入表RVA
		dwNewImportRva = dwNewSectionRVA + dwNewOffset;
		//写入所有的导入表项
		fseek(fp, dwNewFA + dwNewOffset, SEEK_SET);
		fwrite(lpNewImport, dwNewImportSize, 1, fp);
		dwNewOffset += dwNewImportSize;

		//计算文件对齐需要补零的值
		DWORD dwFileAlign = ClacAlignment(dwNewOffset, nFileAlignment) - dwNewOffset;

		for (size_t i = 0; i < dwFileAlign; i++)
		{
			fputc('\0', fp);
		}
		
		//5.添加一个新节表头项
		memset(&ImgNewSection, 0, sizeof(IMAGE_SECTION_HEADER));
		//添加名为.newsec的新节
		strcpy((char*)ImgNewSection.Name, (".NEWSEC"));
		ImgNewSection.VirtualAddress = dwNewSectionRVA;
		ImgNewSection.PointerToRawData = dwNewFA;
		ImgNewSection.Misc.VirtualSize = ClacAlignment(dwNewOffset, nSectionAlignment);
		ImgNewSection.SizeOfRawData = ClacAlignment(dwNewOffset, nFileAlignment);
		ImgNewSection.Characteristics = 0xC0000040;

		//计算新节头的文件偏移
		dwNewSectionOffset = (DWORD)lpFirstSectionHeader -
			(DWORD)m_MapFileStruct.ImageBase + sizeof(IMAGE_SECTION_HEADER)*nSectionNum;
		fseek(fp, dwNewSectionOffset, 0);

		//写入节表头
		fwrite(&ImgNewSection, sizeof(IMAGE_SECTION_HEADER), 1, fp);
		memcpy(&ImgNewSection, lpFirstSectionHeader, sizeof(IMAGE_SECTION_HEADER));
		fseek(fp, (DWORD)lpFirstSectionHeader - (DWORD)m_MapFileStruct.ImageBase, SEEK_SET);
		fwrite(&ImgNewSection, sizeof(IMAGE_SECTION_HEADER), 1, fp);

		//6.更新NT头数据
		memcpy(lpNewNtHeader, lpNtHeader, sizeof(IMAGE_NT_HEADERS));
		int nNewImageSize = lpNtHeader->OptionalHeader.SizeOfImage + ClacAlignment(dwNewOffset, nSectionAlignment);
		lpNewNtHeader->OptionalHeader.SizeOfImage = nNewImageSize;
		lpNewNtHeader->OptionalHeader.DataDirectory[11].Size = 0;
		lpNewNtHeader->OptionalHeader.DataDirectory[11].VirtualAddress = 0;
		lpNewNtHeader->OptionalHeader.DataDirectory[12].Size = 0;
		lpNewNtHeader->OptionalHeader.DataDirectory[12].VirtualAddress = 0;
		lpNewNtHeader->FileHeader.NumberOfSections = nSectionNum + 1;
		lpNewNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress = dwNewImportRva;
		lpNewNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size = dwNewImportSize;

		//写入新的NT头
		fseek(fp, (DWORD)(lpNtHeader)-(DWORD)m_MapFileStruct.ImageBase, SEEK_SET);
		fwrite(lpNewNtHeader, sizeof(IMAGE_NT_HEADERS), 1, fp);

		if (fp != NULL)
		{
			fclose(fp);
			fp = 0;
		}
		UnLoadFile(&m_MapFileStruct);

		//释放扫尾工作
		if (arrINTRva != NULL)
		{
			delete[] arrINTRva;
			arrINTRva = NULL;
		}

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

	MAP_FILE_STRUCT m_MapFileStruct;

	CMenuT<false> m_SystemMenu;
	CEditT<CWindow> m_edtAppExe;
	CEditT<CWindow> m_edtAppDll;
	CEditT<CWindow> m_edtAppFun;
	CButtonT<CWindow> m_btnSelectAppExe;
	CButtonT<CWindow> m_btnAddAppFun;
	CButtonT<CWindow> m_btnImportInject;
	CListViewCtrlT<CWindow> m_ListViewCtrlData;
};
