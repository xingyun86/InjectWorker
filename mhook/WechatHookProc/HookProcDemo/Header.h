// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include <stdio.h>
#include <tchar.h>



// TODO: reference additional headers your program requires here

#include <string.h>
#include <windows.h>
#include <tlhelp32.h>
#include <winternl.h>

#include <map>

#if !defined(_UNICODE) && !defined(UNICODE)
#define tstring std::string
#define __MY__TEXT(quote) quote
#else
#define tstring std::wstring
#define __MY__TEXT(quote) L##quote
#endif

#define _MY_TEXT(x)	__MY__TEXT(x)
#define	_MY_T(x)    __MY__TEXT(x)
#define TSTRING     tstring

typedef std::map<unsigned long, unsigned long> UNSIGNEDLONGMAP;
typedef UNSIGNEDLONGMAP::iterator UNSIGNEDLONGMAPIT;
typedef UNSIGNEDLONGMAP::value_type UNSIGNEDLONGMAPPAIR;

#define STATUS_SUCCESS 0x00UL
#define STATUS_INFO_LENGTH_MISMATCH 0xC0000004

#define SystemHandleInformation 16
#define SE_DEBUG_PRIVILEGE 0x14

typedef enum _OBJECT_INFORMATION_CLASSEX {
	ObjBasicInformation = 0,
	ObjNameInformation,
	ObjTypeInformation,
} OBJECT_INFORMATION_CLASSEX;

typedef enum _PROCESSINFOCLASSEX
{
	ProcessHandleInformation = 20,
}PROCESSINFOCLASSEX;

typedef struct _SYSTEM_HANDLE
{
	ULONG ProcessId;
	BYTE ObjectTypeNumber;
	BYTE Flags;
	USHORT Handle;
	PVOID Object;
	ACCESS_MASK GrantAccess;
}SYSTEM_HANDLE;

typedef struct _SYSTEM_HANDLE_INFORMATION
{
	DWORD HandleCount;
	SYSTEM_HANDLE Handles[1];
}SYSTEM_HANDLE_INFORMATION;

typedef struct _OBJECT_NAME_INFORMATION
{
	UNICODE_STRING ObjectName;
}OBJECT_NAME_INFORMATION;

typedef NTSTATUS(WINAPI *ZwQueryInformationProcessProc)(HANDLE, PROCESSINFOCLASSEX, LPVOID, DWORD, PDWORD);
ZwQueryInformationProcessProc FUN_ZwQueryInformationProcess;

typedef NTSTATUS(WINAPI *ZwQuerySystemInformationProc)(DWORD, PVOID, DWORD, DWORD*);
ZwQuerySystemInformationProc FUN_ZwQuerySystemInformation;

typedef NTSTATUS(WINAPI *ZwQueryObjectProc)(HANDLE, OBJECT_INFORMATION_CLASSEX, PVOID, ULONG, PULONG);
ZwQueryObjectProc FUN_ZwQueryObject;

typedef NTSTATUS(WINAPI *RtlAdjustPrivilegeProc)(DWORD, BOOL, BOOL, PDWORD);
RtlAdjustPrivilegeProc FUN_RtlAdjustPrivilege;

typedef DWORD(WINAPI *ZwSuspendProcessProc)(HANDLE);
ZwSuspendProcessProc FUN_ZwSuspendProcess;

typedef DWORD(WINAPI *ZwResumeProcessProc)(HANDLE);
ZwResumeProcessProc FUN_ZwResumeProcess;


__inline BOOL ElevatePrivileges()
{
	HANDLE hToken;
	TOKEN_PRIVILEGES tkp;
	tkp.PrivilegeCount = 1;
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
		return FALSE;
	LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tkp.Privileges[0].Luid);
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	if (!AdjustTokenPrivileges(hToken, FALSE, &tkp, sizeof(TOKEN_PRIVILEGES), NULL, NULL))
	{
		return FALSE;
	}

	return TRUE;
}

__inline BOOL GetUnDocumentAPI()
{
	FUN_ZwSuspendProcess = (ZwSuspendProcessProc)
		GetProcAddress(GetModuleHandle(_T("ntdll.dll")), "ZwSuspendProcess");

	FUN_ZwQuerySystemInformation = (ZwQuerySystemInformationProc)
		GetProcAddress(GetModuleHandle(_T("ntdll.dll")), "ZwQuerySystemInformation");

	FUN_ZwQueryObject = (ZwQueryObjectProc)
		GetProcAddress(GetModuleHandle(_T("ntdll.dll")), "ZwQueryObject");

	FUN_ZwResumeProcess = (ZwResumeProcessProc)
		GetProcAddress(GetModuleHandle(_T("ntdll.dll")), "ZwResumeProcess");

	FUN_ZwQueryInformationProcess = (ZwQueryInformationProcessProc)
		GetProcAddress(GetModuleHandle(_T("ntdll.dll")), "ZwQueryInformationProcess");

	if ((FUN_ZwSuspendProcess == NULL) || \
		(FUN_ZwQuerySystemInformation == NULL) || \
		(FUN_ZwQueryObject == NULL) || \
		(FUN_ZwResumeProcess == NULL) || \
		(FUN_ZwQueryInformationProcess == NULL))
		return FALSE;

	return TRUE;
}

__inline DWORD GetProcessIdByName(LPCTSTR pName)
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (INVALID_HANDLE_VALUE == hSnapshot)
	{
		return 0;
	}
	PROCESSENTRY32 pe = { sizeof(pe) };
	BOOL fOk;
	for (fOk = Process32First(hSnapshot, &pe); fOk; fOk = Process32Next(hSnapshot, &pe))
	{
		if (!_tcsicmp(pe.szExeFile, pName))
		{
			CloseHandle(hSnapshot);
			return pe.th32ProcessID;
		}
	}
	return 0;
}

__inline size_t GetProcessIdByName(UNSIGNEDLONGMAP & ulmap, LPCTSTR pName)
{
	PROCESSENTRY32 pe = { sizeof(pe) };
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (INVALID_HANDLE_VALUE != hSnapshot)
	{
		for (BOOL fOk = Process32First(hSnapshot, &pe); fOk; fOk = Process32Next(hSnapshot, &pe))
		{
			if (!_tcsicmp(pe.szExeFile, pName))
			{
				ulmap.insert(UNSIGNEDLONGMAPPAIR(pe.th32ProcessID, pe.th32ProcessID));
			}
		}

		CloseHandle(hSnapshot);
	}
	return ulmap.size();
}
__inline size_t GetMaxProcessIdByName(UNSIGNEDLONGMAP & ulmap, LPCTSTR pName)
{
	PROCESSENTRY32 pe = { sizeof(pe) };
	DWORD dwProcessID = 0;
	LARGE_INTEGER liCreateTime = { 0 };
	LARGE_INTEGER liCreateTimeTemp = { 0 };
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (INVALID_HANDLE_VALUE != hSnapshot)
	{
		for (BOOL fOk = Process32First(hSnapshot, &pe); fOk; fOk = Process32Next(hSnapshot, &pe))
		{
			if (!_tcsicmp(pe.szExeFile, pName))
			{
				HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe.th32ProcessID);
				if (hProcess)
				{
					FILETIME ftCreation = { 0 };
					FILETIME ftExit = { 0 };
					FILETIME ftKernel = { 0 };
					FILETIME ftUser = { 0 };
					if (GetProcessTimes(hProcess, &ftCreation, &ftExit, &ftKernel, &ftUser))
					{
						liCreateTimeTemp.HighPart = ftCreation.dwHighDateTime;
						liCreateTimeTemp.LowPart = ftCreation.dwLowDateTime;
						if (liCreateTime.QuadPart < liCreateTimeTemp.QuadPart)
						{
							dwProcessID = pe.th32ProcessID;
							memcpy(&liCreateTime, &liCreateTimeTemp, sizeof(LARGE_INTEGER));
						}
					}
					CloseHandle(hProcess);
				}
			}
		}
		CloseHandle(hSnapshot);

		if (dwProcessID > 0)
		{
			ulmap.insert(UNSIGNEDLONGMAPPAIR(dwProcessID, dwProcessID));
		}
	}
	return ulmap.size();
}


//获取程序文件路径
__inline tstring GetProgramPath()
{
	tstring tstrFilePath = _T("");
	_TCHAR tFilePath[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, tFilePath, MAX_PATH);
	if (*tFilePath)
	{
		_TCHAR * pFound = _tcsrchr(tFilePath, _T('\\'));
		if (*(++pFound))
		{
			*pFound = _T('\0');
		}
		tstrFilePath = tFilePath;
	}
	return tstrFilePath;
}

//根据进程ID终止进程
__inline void TerminateProcessByProcessId(DWORD dwProcessId)
{
	DWORD dwExitCode = 0;
	HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, dwProcessId);
	if (hProcess)
	{
		GetExitCodeProcess(hProcess, &dwExitCode);
		TerminateProcess(hProcess, dwExitCode);
		CloseHandle(hProcess);
		hProcess = 0;
	}
}

//传入应用程序文件名称、参数、启动类型及等待时间启动程序
typedef enum LaunchType {
	LTYPE_0 = 0, //立即
	LTYPE_1 = 1, //直等
	LTYPE_2 = 2, //延迟(设定等待时间)
}LAUNCHTYPE;

//传入应用程序文件名称、参数、启动类型及等待时间启动程序
__inline  BOOL LaunchAppProg(tstring tsAppProgName, tstring tsArguments = _T(""), bool bNoUI = true, LAUNCHTYPE type = LTYPE_0, DWORD dwWaitTime = WAIT_TIMEOUT)
{
	BOOL bRet = FALSE;
	STARTUPINFO si = { 0 };
	PROCESS_INFORMATION pi = { 0 };
	DWORD dwCreateFlags = CREATE_NO_WINDOW;
	LPTSTR lpArguments = NULL;

	if (tsArguments.length())
	{
		lpArguments = (LPTSTR)tsArguments.c_str();
	}

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	if (!bNoUI)
	{
		dwCreateFlags = 0;
	}

	// Start the child process.
	bRet = CreateProcess(tsAppProgName.c_str(),   // No module name (use command line)
		lpArguments,        // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		dwCreateFlags,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory
		&si,            // Pointer to STARTUPINFO structure
		&pi);           // Pointer to PROCESS_INFORMATION structure
	if (bRet)
	{
		switch (type)
		{
		case LTYPE_0:
		{
			// No wait until child process exits.
		}
		break;
		case LTYPE_1:
		{
			// Wait until child process exits.
			WaitForSingleObject(pi.hProcess, INFINITE);
		}
		break;
		case LTYPE_2:
		{
			// Wait until child process exits.
			WaitForSingleObject(pi.hProcess, dwWaitTime);
		}
		break;
		default:
			break;
		}

		// Close process and thread handles.
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);

		// Exit process.
		//TerminateProcessByProcessId(pi.dwProcessId);
	}
	else
	{
		//DEBUG_TRACE(_T("CreateProcess failed (%d).\n"), GetLastError());
	}
	return bRet;
}

//传入应用程序文件名称、参数、启动类型及等待时间启动程序
__inline DWORD LaunchProgram(tstring tsAppProgName, tstring tsArguments = _T(""), DWORD dwFlags = CREATE_NEW_CONSOLE, LPVOID lpEnvironment = NULL, LPCTSTR lpCurrentDirectory = NULL, LAUNCHTYPE type = LTYPE_0, DWORD dwWaitTime = WAIT_TIMEOUT)
{
	BOOL bRet = FALSE;
	DWORD dwRet = (-1L);
	STARTUPINFO si = { 0 };
	PROCESS_INFORMATION pi = { 0 };
	DWORD dwCreateFlags = dwFlags;
	LPTSTR lpArguments = NULL;

	if (tsArguments.length())
	{
		lpArguments = (LPTSTR)tsArguments.c_str();
	}

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	// Start the child process.
	bRet = CreateProcess(tsAppProgName.c_str(),   // No module name (use command line)
		lpArguments,        // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		dwCreateFlags,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory
		&si,            // Pointer to STARTUPINFO structure
		&pi);           // Pointer to PROCESS_INFORMATION structure
	if (bRet)
	{
		switch (type)
		{
		case LTYPE_0:
		{
			// No wait until child process exits.
		}
		break;
		case LTYPE_1:
		{
			// Wait until child process exits.
			WaitForSingleObject(pi.hProcess, INFINITE);
		}
		break;
		case LTYPE_2:
		{
			// Wait until child process exits.
			WaitForSingleObject(pi.hProcess, dwWaitTime);
		}
		break;
		default:
			break;
		}
		dwRet = pi.dwProcessId;
		// Close process and thread handles.
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);

		// Exit process.
		//TerminateProcessByProcessId(pi.dwProcessId);
	}
	else
	{
		//DEBUG_TRACE(_T("CreateProcess failed (%d).\n"), GetLastError());
	}
	return dwRet;
}

__inline int t_main(int argc, char* argv[])
{
	HANDLE duplicateHnd = 0;
	HANDLE sourceHnd = 0;
	SIZE_T procHndNum = 0;
	ULONG pidCount = 0;
	SYSTEM_HANDLE* currnetHnd = 0;
	DWORD buffLen = sizeof(SYSTEM_HANDLE_INFORMATION);
	NTSTATUS status = 0;
	SYSTEM_HANDLE_INFORMATION* buff = (SYSTEM_HANDLE_INFORMATION*)malloc(buffLen);
	_TCHAR tFileName[MAX_PATH] = _T("WeChat.exe");
	WCHAR wTypeObjName[MAX_PATH] = L"Mutant";
	WCHAR *wMutexList[] = {
		L"_WeChat_App_Instance_Identity_Mutex_Name",
		L"WeChat_GlobalConfig_Multi_Process_Mutex",
	};
	INT nMutexNum = sizeof(wMutexList) / sizeof(WCHAR *);
	INT nMutexIdx = 0;
	OBJECT_NAME_INFORMATION* objNameInfo = 0;
	OBJECT_NAME_INFORMATION* objTypeInfo = 0;
	UNSIGNEDLONGMAP ulmap;

	pidCount = GetMaxProcessIdByName(ulmap, tFileName);
	if (pidCount <= 0)
	{
		goto __LEAVE_CLEAN__;
	}

	if ((ElevatePrivileges() == FALSE) || (GetUnDocumentAPI() == FALSE))
	{
		goto __LEAVE_CLEAN__;
	}

	objNameInfo = (OBJECT_NAME_INFORMATION*)malloc(0x1000);
	objTypeInfo = (OBJECT_NAME_INFORMATION*)malloc(0x1000);

	sourceHnd = OpenProcess(PROCESS_ALL_ACCESS | PROCESS_DUP_HANDLE | PROCESS_SUSPEND_RESUME, FALSE, ulmap.begin()->first);
	(FUN_ZwSuspendProcess)(sourceHnd);
	(FUN_ZwQueryInformationProcess)(sourceHnd, ProcessHandleInformation, &procHndNum, sizeof(SIZE_T), NULL);
	//进程有效句柄从sizeof(SIZE_T)开始,每次以sizeof(SIZE_T)递增
	for (int idx = 0, hndNum = sizeof(SIZE_T); idx < procHndNum && hndNum < 16384; hndNum += sizeof(SIZE_T))
	{
		//判断是否为有效句柄，返回TRUE，就是有效句柄
		if (!DuplicateHandle(sourceHnd,
			(HANDLE)hndNum,
			GetCurrentProcess(),
			&duplicateHnd, 0, FALSE, DUPLICATE_SAME_ACCESS))
		{
			continue;
		}
		else
		{
			memset(objNameInfo, 0, 0x1000);
			memset(objTypeInfo, 0, 0x1000);

			FUN_ZwQueryObject((HANDLE)duplicateHnd, ObjNameInformation, objNameInfo, 0x1000, NULL);
			FUN_ZwQueryObject((HANDLE)duplicateHnd, ObjTypeInformation, objTypeInfo, 0x1000, NULL);

			//找到互斥体 比较名字
			if (_wcsicmp(objTypeInfo->ObjectName.Buffer, wTypeObjName) == 0)
			{
				if (objNameInfo->ObjectName.Buffer)
				{
					for (int n = 0; n < nMutexNum; n++)
					{
						if (wcsstr(objNameInfo->ObjectName.Buffer, wMutexList[n]) != 0)
						{
							CloseHandle(duplicateHnd);

							if (DuplicateHandle(sourceHnd,
								(HANDLE)hndNum,
								GetCurrentProcess(),
								&duplicateHnd, 0, FALSE, DUPLICATE_CLOSE_SOURCE))
							{
								nMutexIdx++;
								break;
							}
						}
					}
				}
			}
			CloseHandle(duplicateHnd);
			idx++;

			if (nMutexIdx >= nMutexNum)
			{
				nMutexIdx = 0;
				break;
			}
		}
	}
	(FUN_ZwResumeProcess)(sourceHnd);

__LEAVE_CLEAN__:

	if (objTypeInfo)
	{
		free(objTypeInfo);
		objTypeInfo = 0;
	}
	if (objNameInfo)
	{
		free(objNameInfo);
		objNameInfo = 0;
	}

	return 0;
}


//进程快照（枚举各进程）
__inline BOOL GetPidByProcessName(LPCTSTR lpszProcessName, SIZE_T &stPid)
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

/********************************************************************************************************/

//注入DLL到远程进程
__inline BOOL InjectDllToRemoteProcess(const _TCHAR* lpDllName, const _TCHAR* lpPid, const _TCHAR* lpProcName)
{
	SIZE_T stPid = 0;

	if ((ElevatePrivileges() == FALSE) || (GetUnDocumentAPI() == FALSE))
	{
		return FALSE;
	}

	if (_tcsstr(lpProcName, _T("\\")) || _tcsstr(lpProcName, _T("/")))
	{
		//stPid = LaunchProgram(lpProcName, _T(""), CREATE_SUSPENDED);
		stPid = LaunchProgram(lpProcName, _T(""));
	}

	if (stPid <= 0)
	{
		if (NULL == lpPid || 0 == _tcslen(lpPid))
		{
			if (NULL != lpProcName && 0 != _tcslen(lpProcName))
			{
				if (!GetPidByProcessName(lpProcName, stPid))
				{
					return FALSE;
				}
			}
			else
			{
				return FALSE;
			}
		}
		else
		{
			stPid = _ttoi(lpPid);
		}
	}

	//根据Pid得到进程句柄(注意必须权限)
	HANDLE hRemoteProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_SUSPEND_RESUME, FALSE, stPid);
	if (INVALID_HANDLE_VALUE == hRemoteProcess)
	{
		return FALSE;
	}
	
	//(FUN_ZwResumeProcess)(hRemoteProcess);
	//(FUN_ZwSuspendProcess)(hRemoteProcess);

	//计算DLL路径名需要的内存空间
	SIZE_T stSize = (1 + _tcslen(lpDllName)) * sizeof(_TCHAR);

	//使用VirtualAllocEx函数在远程进程的内存地址空间分配DLL文件名缓冲区,成功返回分配内存的首地址.
	LPVOID lpRemoteBuff = (char *)VirtualAllocEx(hRemoteProcess, NULL, stSize, MEM_COMMIT, PAGE_READWRITE);
	if (NULL == lpRemoteBuff)
	{
		CloseHandle(hRemoteProcess);
		return FALSE;
	}

	//使用WriteProcessMemory函数将DLL的路径名复制到远程进程的内存空间,成功返回TRUE.
	SIZE_T stHasWrite = 0;
	BOOL bRet = WriteProcessMemory(hRemoteProcess, lpRemoteBuff, lpDllName, stSize, &stHasWrite);
	if (!bRet || stHasWrite != stSize)
	{
		VirtualFreeEx(hRemoteProcess, lpRemoteBuff, stSize, MEM_COMMIT);
		CloseHandle(hRemoteProcess);
		return FALSE;
	}

	//创建一个在其它进程地址空间中运行的线程(也称:创建远程线程),成功返回新线程句柄.
	//注意:进程句柄必须具备PROCESS_CREATE_THREAD, PROCESS_QUERY_INFORMATION, PROCESS_VM_OPERATION, PROCESS_VM_WRITE,和PROCESS_VM_READ访问权限
	DWORD dwRemoteThread = 0;
	LPTHREAD_START_ROUTINE pfnLoadLibrary = (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(_T("Kernel32")), "LoadLibraryW");
	HANDLE hRemoteThread = CreateRemoteThread(hRemoteProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pfnLoadLibrary, lpRemoteBuff, 0, &dwRemoteThread);
	if (INVALID_HANDLE_VALUE == hRemoteThread)
	{
		VirtualFreeEx(hRemoteProcess, lpRemoteBuff, stSize, MEM_COMMIT);
		CloseHandle(hRemoteProcess);
		return FALSE;
	}

	//注入成功释放句柄
	WaitForSingleObject(hRemoteThread, INFINITE);

	//(FUN_ZwResumeProcess)(hRemoteProcess);

	CloseHandle(hRemoteThread);
	CloseHandle(hRemoteProcess);
	
	return TRUE;
}
