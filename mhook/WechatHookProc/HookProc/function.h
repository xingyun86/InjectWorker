// function.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//
#pragma once
#ifndef __FUNCTION_H_
#define __FUNCTION_H_

#include <tchar.h>

#include <winsock2.h>
#include <winhttp.h>

#include <map>
#include <string>
// TODO: reference additional headers your program requires here

#define VOIDVOIDMAP_INIT(NAME)  {&True##NAME, Hook##NAME}
#define STRINGVOIDVOIDMAP_INIT(NAME)  {#NAME, {VOIDVOIDMAP_INIT(NAME)}}
#define STRINGSTRINGVOIDVOIDMAP_INIT(CORE, NAME)  {#CORE, {STRINGVOIDVOIDMAP_INIT(NAME)}}

#define FUNC_INIT(CORE, NAME) static TYPE_##NAME True##NAME = (TYPE_##NAME)GetProcAddress(GetModuleHandle(_T(#CORE)), #NAME);

typedef std::map<void *, void *> VOIDVOIDMAP;
typedef VOIDVOIDMAP::iterator VOIDVOIDMAPIT;
typedef VOIDVOIDMAP::value_type VOIDVOIDMAPPAIR;

typedef std::map<std::string, VOIDVOIDMAP> STRINGVOIDVOIDMAP;
typedef STRINGVOIDVOIDMAP::iterator STRINGVOIDVOIDMAPIT;
typedef STRINGVOIDVOIDMAP::value_type STRINGVOIDVOIDMAPPAIR;

typedef std::map<std::string, STRINGVOIDVOIDMAP> STRINGSTRINGVOIDVOIDMAP;
typedef STRINGSTRINGVOIDVOIDMAP::iterator STRINGSTRINGVOIDVOIDMAPIT;
typedef STRINGSTRINGVOIDVOIDMAP::value_type STRINGSTRINGVOIDVOIDMAPPAIR;

typedef HANDLE(WINAPI *TYPE_CreateFileW)(
	_In_     LPCTSTR               lpFileName,
	_In_     DWORD                 dwDesiredAccess,
	_In_     DWORD                 dwShareMode,
	_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	_In_     DWORD                 dwCreationDisposition,
	_In_     DWORD                 dwFlagsAndAttributes,
	_In_opt_ HANDLE                hTemplateFile
	);


HANDLE WINAPI HookCreateFileW(
	_In_     LPCTSTR               lpFileName,
	_In_     DWORD                 dwDesiredAccess,
	_In_     DWORD                 dwShareMode,
	_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	_In_     DWORD                 dwCreationDisposition,
	_In_     DWORD                 dwFlagsAndAttributes,
	_In_opt_ HANDLE                hTemplateFile
);
typedef BOOL(WINAPI *TYPE_WriteFile)(
	_In_        HANDLE       hFile,
	_In_        LPCVOID      lpBuffer,
	_In_        DWORD        nNumberOfBytesToWrite,
	_Out_opt_   LPDWORD      lpNumberOfBytesWritten,
	_Inout_opt_ LPOVERLAPPED lpOverlapped
	);


BOOL WINAPI HookWriteFile(
	_In_        HANDLE       hFile,
	_In_        LPCVOID      lpBuffer,
	_In_        DWORD        nNumberOfBytesToWrite,
	_Out_opt_   LPDWORD      lpNumberOfBytesWritten,
	_Inout_opt_ LPOVERLAPPED lpOverlapped
);
typedef int (WSAAPI *TYPE_send)(
	_In_       SOCKET s,
	_In_ const char   *buf,
	_In_       int    len,
	_In_       int    flags
	);

int WSAAPI Hooksend(
	_In_       SOCKET s,
	_In_ const char   *buf,
	_In_       int    len,
	_In_       int    flags
);
typedef int (WSAAPI *TYPE_recv)(
	_In_       SOCKET s,
	_In_ const char   *buf,
	_In_       int    len,
	_In_       int    flags
	);

int WSAAPI Hookrecv(
	_In_       SOCKET s,
	_In_ const char   *buf,
	_In_       int    len,
	_In_       int    flags
);

typedef BOOL(WINAPI *TYPE_WinHttpGetIEProxyConfigForCurrentUser)(
	_Inout_ WINHTTP_CURRENT_USER_IE_PROXY_CONFIG *pProxyConfig
	);

int WSAAPI HookWinHttpGetIEProxyConfigForCurrentUser(
	_Inout_ WINHTTP_CURRENT_USER_IE_PROXY_CONFIG *pProxyConfig
);
typedef LONG (WINAPI *TYPE_RegOpenKeyExW)(
	_In_     HKEY    hKey,
	_In_opt_ LPCTSTR lpSubKey,
	_In_     DWORD   ulOptions,
	_In_     REGSAM  samDesired,
	_Out_    PHKEY   phkResult
);

LONG WINAPI HookRegOpenKeyExW(
	_In_     HKEY    hKey,
	_In_opt_ LPCTSTR lpSubKey,
	_In_     DWORD   ulOptions,
	_In_     REGSAM  samDesired,
	_Out_    PHKEY   phkResult
);
typedef LONG (WINAPI *TYPE_RegOpenKeyW)(
	_In_     HKEY    hKey,
	_In_opt_ LPCTSTR lpSubKey,
	_Out_    PHKEY   phkResult
);

LONG WINAPI HookRegOpenKeyW(
	_In_     HKEY    hKey,
	_In_opt_ LPCTSTR lpSubKey,
	_Out_    PHKEY   phkResult
);
typedef LONG(WINAPI *TYPE_RegQueryValueExW)(
	_In_        HKEY    hKey,
	_In_opt_    LPCTSTR lpValueName,
	_Reserved_  LPDWORD lpReserved,
	_Out_opt_   LPDWORD lpType,
	_Out_opt_   LPBYTE  lpData,
	_Inout_opt_ LPDWORD lpcbData
	);

LONG WINAPI HookRegQueryValueExW(
	_In_        HKEY    hKey,
	_In_opt_    LPCTSTR lpValueName,
	_Reserved_  LPDWORD lpReserved,
	_Out_opt_   LPDWORD lpType,
	_Out_opt_   LPBYTE  lpData,
	_Inout_opt_ LPDWORD lpcbData
);
typedef HINSTANCE(*TYPE_ShellExecuteW)(
	_In_opt_ HWND    hwnd,
	_In_opt_ LPCTSTR lpOperation,
	_In_     LPCTSTR lpFile,
	_In_opt_ LPCTSTR lpParameters,
	_In_opt_ LPCTSTR lpDirectory,
	_In_     INT     nShowCmd
	);

HINSTANCE HookShellExecuteW(
	_In_opt_ HWND    hwnd,
	_In_opt_ LPCTSTR lpOperation,
	_In_     LPCTSTR lpFile,
	_In_opt_ LPCTSTR lpParameters,
	_In_opt_ LPCTSTR lpDirectory,
	_In_     INT     nShowCmd
);

FUNC_INIT(KERNEL32, CreateFileW)
FUNC_INIT(KERNEL32, WriteFile)
FUNC_INIT(WS2_32, send)
FUNC_INIT(WS2_32, recv)
FUNC_INIT(WINHTTP, WinHttpGetIEProxyConfigForCurrentUser)
FUNC_INIT(ADVAPI32, RegOpenKeyW)
FUNC_INIT(ADVAPI32, RegOpenKeyExW)
FUNC_INIT(ADVAPI32, RegQueryValueExW)
FUNC_INIT(SHELL32, ShellExecuteW)

static STRINGSTRINGVOIDVOIDMAP G_SSVVM = {
	//{ _T("KERNEL32"), { STRINGVOIDVOIDMAP_INIT(CreateFileW), STRINGVOIDVOIDMAP_INIT(WriteFile)}},
	//{ _T("WS2_32"), { STRINGVOIDVOIDMAP_INIT(send), STRINGVOIDVOIDMAP_INIT(recv)}},
	//STRINGSTRINGVOIDVOIDMAP_INIT(KERNEL32, CreateFileW),
	//STRINGSTRINGVOIDVOIDMAP_INIT(WINHTTP, WinHttpGetIEProxyConfigForCurrentUser),
	//STRINGSTRINGVOIDVOIDMAP_INIT(ADVAPI32, RegOpenKeyExW),
	//STRINGSTRINGVOIDVOIDMAP_INIT(ADVAPI32, RegQueryValueExW),
	//STRINGSTRINGVOIDVOIDMAP_INIT(SHELL32, ShellExecuteW),
};

#endif // !__FUNCTION_H_
