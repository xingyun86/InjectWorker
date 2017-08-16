// function.cpp : source file that includes just the standard includes
// HookProc.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information
#include "stdafx.h"
#include "function.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file
HANDLE WINAPI HookCreateFileW(
	_In_     LPCTSTR               lpFileName,
	_In_     DWORD                 dwDesiredAccess,
	_In_     DWORD                 dwShareMode,
	_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	_In_     DWORD                 dwCreationDisposition,
	_In_     DWORD                 dwFlagsAndAttributes,
	_In_opt_ HANDLE                hTemplateFile
)
{
	DebugPrintString(_T("Enter CreateFileW"));
	DebugPrintString(tstring(_T("FileName=")) + lpFileName + _T("==") + STRING_FROM_HEX((unsigned long)TrueCreateFileW));
	
	return TrueCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

BOOL WINAPI HookWriteFile(
	_In_        HANDLE       hFile,
	_In_        LPCVOID      lpBuffer,
	_In_        DWORD        nNumberOfBytesToWrite,
	_Out_opt_   LPDWORD      lpNumberOfBytesWritten,
	_Inout_opt_ LPOVERLAPPED lpOverlapped
)
{
	DebugPrintString(_T("Enter WriteFile"));
	DebugPrintString(tstring(_T("WriteSize=")) + STRING_FROM_LONG(nNumberOfBytesToWrite));
	if (nNumberOfBytesToWrite > 0)
	{
		DebugPrintString(tstring(_T("WrittenSize=")) + STRING_FROM_LONG(*lpNumberOfBytesWritten) + _T("WriteData=") + (_TCHAR *)lpBuffer);
	}
	return TrueWriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
}

int WSAAPI Hooksend(
	_In_       SOCKET s,
	_In_ const char   *buf,
	_In_       int    len,
	_In_       int    flags
)
{
	DebugPrintString(_T("Enter send"));
	DebugPrintString(_T("Sendsize=") + STRING_FROM_INT(len));
	if (len > 0)
	{
		DebugPrintString(tstring(_T("Senddata=%s")) + (_TCHAR*)buf);
	}
	return Truesend(s, buf, len, flags);
}

int WSAAPI Hookrecv(
	_In_       SOCKET s,
	_In_ const char   *buf,
	_In_       int    len,
	_In_       int    flags
)
{
	DebugPrintString(_T("Enter recv"));
	DebugPrintString(_T("Recvsize=") + STRING_FROM_INT(len));
	int nRecvLen = Truerecv(s, buf, len, flags);
	if (nRecvLen > 0)
	{
		DebugPrintString(_T("Recvedsize=") + STRING_FROM_INT(nRecvLen) + _T("Recvdata=") + (_TCHAR*)buf);
	}
	return nRecvLen;
}

int WSAAPI HookWinHttpGetIEProxyConfigForCurrentUser(
	_Inout_ WINHTTP_CURRENT_USER_IE_PROXY_CONFIG *pProxyConfig
)
{
	DebugPrintString(_T("Enter WinHttpGetIEProxyConfigForCurrentUser"));
	BOOL bResult = FALSE;
	bResult = TrueWinHttpGetIEProxyConfigForCurrentUser(pProxyConfig);
	if (bResult)
	{
		DebugPrintString(tstring(_T("fAutoDetect=")) + STRING_FROM_INT(pProxyConfig->fAutoDetect) + _T(",") + WToT(pProxyConfig->lpszAutoConfigUrl) + _T(",") + WToT(pProxyConfig->lpszProxy) + _T(",") + WToT(pProxyConfig->lpszProxyBypass));
	}
	//pProxyConfig->lpszProxy = _T("127.0.0.1:3128");
	//bResult = TRUE;
	return bResult;
}
LONG WINAPI HookRegOpenKeyW(
	_In_     HKEY    hKey,
	_In_opt_ LPCTSTR lpSubKey,
	_Out_    PHKEY   phkResult
) 
{
	LONG lResult = 0;

	lResult = TrueRegOpenKeyW(hKey, lpSubKey, phkResult);

	return lResult;
}
LONG WINAPI HookRegOpenKeyExW(
	_In_     HKEY    hKey,
	_In_opt_ LPCTSTR lpSubKey,
	_In_     DWORD   ulOptions,
	_In_     REGSAM  samDesired,
	_Out_    PHKEY   phkResult
)
{
	LONG lResult = 0;

	lResult = TrueRegOpenKeyExW(hKey, lpSubKey, ulOptions, samDesired, phkResult);

	return lResult;
}
LONG WINAPI HookRegQueryValueExW(
	_In_        HKEY    hKey,
	_In_opt_    LPCTSTR lpValueName,
	_Reserved_  LPDWORD lpReserved,
	_Out_opt_   LPDWORD lpType,
	_Out_opt_   LPBYTE  lpData,
	_Inout_opt_ LPDWORD lpcbData
)
{
	DebugPrintString(_T("Enter RegQueryValueExW"));
	DebugPrintString(tstring(_T("lpValueName=")) + lpValueName);
	LONG lResult = 0;
	lResult = TrueRegQueryValueExW(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
	if (lResult)
	{
		if (lpData)
		{
			DebugPrintString(tstring(_T("lpData=")) + (const _TCHAR *)lpData);
		}
	}

	return lResult;
}

HINSTANCE HookShellExecuteW(
	_In_opt_ HWND    hwnd,
	_In_opt_ LPCTSTR lpOperation,
	_In_     LPCTSTR lpFile,
	_In_opt_ LPCTSTR lpParameters,
	_In_opt_ LPCTSTR lpDirectory,
	_In_     INT     nShowCmd
)
{
	DebugPrintString(_T("Enter ShellExecuteW"));
	HINSTANCE hInstance = 0;
	DebugPrintString(tstring(lpOperation) + _T(",") + lpFile + _T(",") + lpParameters + _T(",") + lpDirectory);
	hInstance = TrueShellExecuteW(hwnd, lpOperation, lpFile, lpParameters, lpDirectory, nShowCmd);

	return hInstance;
}
