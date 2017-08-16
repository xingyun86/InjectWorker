// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <tchar.h>

#include <string>


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


//进程快照（枚举各进程）
static __inline BOOL GetPidByProcessName(LPCTSTR lpszProcessName, SIZE_T &stPid)
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (INVALID_HANDLE_VALUE == hSnapshot)
	{
		return FALSE;
	}

	PROCESSENTRY32 pe;
	pe.dwSize = sizeof(PROCESSENTRY32);
	if (!Process32First(hSnapshot, &pe))
	{
		::CloseHandle(hSnapshot);
		return FALSE;
	}

	while (Process32Next(hSnapshot, &pe))
	{
		if (!_tcsicmp(lpszProcessName, pe.szExeFile))
		{
			::CloseHandle(hSnapshot);
			stPid = pe.th32ProcessID;
			return TRUE;
		}
	}

	::CloseHandle(hSnapshot);

	return FALSE;
}
