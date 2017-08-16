// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <tchar.h>

#include <string>

#define MAX_BASE_SIZE   128
#define MAX_NAME_SIZE   256
#define MAX_CMDS_SIZE   1024
#define MAX_DATA_SIZE   16384

// TODO: reference additional headers your program requires here
//获取程序文件路径
static __inline std::string GetProgramPath(HINSTANCE hInstance = NULL)
{
	std::string tstrFilePath = ("");
	char tFilePath[MAX_PATH] = { 0 };
	GetModuleFileNameA(hInstance, tFilePath, MAX_PATH);
	if (*tFilePath)
	{
		char * pFound = strrchr(tFilePath, ('\\'));
		if (*(++pFound))
		{
			*pFound = ('\0');
		}
		tstrFilePath = tFilePath;
	}
	return tstrFilePath;
}

//初始化调试窗口显示
__inline static void InitDebugConsole()
{
	if (!AllocConsole())
	{
		_TCHAR tErrorInfos[MAX_DATA_SIZE] = { 0 };
		_sntprintf(tErrorInfos, sizeof(tErrorInfos) / sizeof(_TCHAR), _T("控制台生成失败! 错误代码:0x%X。"), GetLastError());
		MessageBox(NULL, tErrorInfos, _T("错误提示"), 0);
		return;
	}
	SetConsoleTitle(_T("TraceDebugWindow"));

	_tfreopen(_T("CONIN$"), _T("rb"), stdin);
	_tfreopen(_T("CONOUT$"), _T("wb"), stdout);
	_tfreopen(_T("CONERR$"), _T("wb"), stderr);
	_tsetlocale(LC_ALL, _T("chs"));
}

//释放掉调试窗口显示
__inline static void ExitDebugConsole()
{
	fclose(stderr);
	fclose(stdout);
	fclose(stdin);
	FreeConsole();
}
