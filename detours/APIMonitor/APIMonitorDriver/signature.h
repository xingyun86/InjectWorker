
#define FUNC_INIT(CORE, NAME) static TYPE_##NAME* TRUE_##NAME = (TYPE_##NAME*)GetProcAddress(GetModuleHandle(_T(#CORE)), #NAME);

//#define FUNC_LOCAL_INIT(CORE, NAME) TYPE_##NAME* TRUE_##NAME = 0; { HMODULE hModule = GetModuleHandle(_T(#CORE)); if(hModule){ TRUE_##NAME = (TYPE_##NAME*)GetProcAddress(hModule, #NAME);}}

VOID * FUNC_LOCAL_INIT(_TCHAR * ptModuleName, CHAR * pFunctionName)
{
	VOID * pFunc = 0;
	HMODULE hModule = GetModuleHandle(ptModuleName);
	printf("hModule=0x%x====\n", hModule);
	if (hModule) 
	{ 
		pFunc = (void *)::GetProcAddress(hModule, pFunctionName);
		printf("pFunc=0x%x====\n", pFunc);
	}
	return pFunc;
}

BOOL ReadProcessMemoryValue(
	__in      HANDLE hProcess,
	__in      LPCVOID lpBaseAddress,
	__out_bcount_part(nSize, *lpNumberOfBytesRead) LPVOID lpBuffer,
	__in      SIZE_T nSize,
	__out_opt SIZE_T * lpNumberOfBytesRead
)
{
	return ReadProcessMemory(hProcess, lpBaseAddress, lpBuffer, nSize, lpNumberOfBytesRead);
}

typedef VOID WINAPI TYPE_ExitProcess(
	_In_ UINT uExitCode
);
FUNC_INIT(KERNEL32, ExitProcess)
VOID WINAPI HOOK_ExitProcess(UINT uExitCode)
{
	COMMONPARAMSVECTOR cpv;
	CCommonParams cp;

	cp.type = ET_ARGS_PCHAR;
	cp.data = "ExitProcess";
	cpv.push_back(cp);
	cp.type = ET_ARGS_PCHAR;
	cp.data = STRING_FROM_HEX(uExitCode);
	cpv.push_back(cp);
	log(&cpv);

	return TRUE_ExitProcess(uExitCode);
}

typedef VOID WINAPI TYPE_Sleep(
	_In_ DWORD dwMilliseconds
);
FUNC_INIT(KERNEL32, Sleep)
VOID WINAPI HOOK_Sleep(DWORD dwMilliseconds)
{
	COMMONPARAMSVECTOR cpv;
	CCommonParams cp;

	cp.type = ET_ARGS_PCHAR;
	cp.data = "Sleep";
	cpv.push_back(cp);
	cp.type = ET_ARGS_PCHAR;
	cp.data = STRING_FROM_HEX(dwMilliseconds);
	cpv.push_back(cp);
	log(&cpv);

	return TRUE_Sleep(dwMilliseconds);
}

typedef FARPROC WINAPI TYPE_GetProcAddress(
	_In_ HMODULE hModule,
	_In_ LPCSTR  lpProcName
);
FUNC_INIT(KERNEL32, GetProcAddress)
FARPROC WINAPI HOOK_GetProcAddress(HMODULE hModule, LPCSTR lpProcName)
{
	FARPROC fResult = 0;
	COMMONPARAMSVECTOR cpv;
	CCommonParams cp;
	
	fResult = TRUE_GetProcAddress(hModule, lpProcName);

	cp.type = ET_ARGS_PCHAR;
	cp.data = "GetProcAddress";
	cpv.push_back(cp);
	cp.type = ET_ARGS_PCHAR;
	cp.data = lpProcName;
	cpv.push_back(cp);
	log(&cpv);

	return fResult;
}

typedef HMODULE WINAPI TYPE_LoadLibraryA(
	_In_ LPCSTR lpFileName
);
FUNC_INIT(KERNEL32, LoadLibraryA)
HMODULE WINAPI HOOK_LoadLibraryA(LPCSTR lpFileName)
{
	HMODULE hResult = 0;
	COMMONPARAMSVECTOR cpv;
	CCommonParams cp;
	
	hResult = TRUE_LoadLibraryA(lpFileName);

	cp.type = ET_ARGS_PCHAR;
	cp.data = "LoadLibraryA";
	cpv.push_back(cp);
	cp.type = ET_ARGS_PCHAR;
	cp.data = lpFileName;
	cpv.push_back(cp);
	log(&cpv);

	return hResult;
}

typedef HMODULE WINAPI TYPE_LoadLibraryW(
	_In_ LPCWSTR lpFileName
);
FUNC_INIT(KERNEL32, LoadLibraryW)
HMODULE WINAPI HOOK_LoadLibraryW(LPCWSTR lpFileName)
{
	HMODULE hResult = 0;
	COMMONPARAMSVECTOR cpv;
	CCommonParams cp;

	hResult = TRUE_LoadLibraryW(lpFileName);

	cp.type = ET_ARGS_PCHAR;
	cp.data = "LoadLibraryW";
	cpv.push_back(cp);
	cp.type = ET_ARGS_PCHAR;
	cp.data = WToA(lpFileName);
	cpv.push_back(cp);
	log(&cpv);

	return hResult;
}

typedef LONG WINAPI TYPE_RegOpenKeyExW(
	_In_     HKEY    hKey,
	_In_opt_ LPCWSTR lpSubKey,
	_In_     DWORD   ulOptions,
	_In_     REGSAM  samDesired,
	_Out_    PHKEY   phkResult
	);
FUNC_INIT(ADVAPI32, RegOpenKeyExW)
LONG WINAPI HOOK_RegOpenKeyExW(
	_In_     HKEY    hKey,
	_In_opt_ LPCWSTR lpSubKey,
	_In_     DWORD   ulOptions,
	_In_     REGSAM  samDesired,
	_Out_    PHKEY   phkResult
)
{
	LONG lResult = 0;

	lResult = TRUE_RegOpenKeyExW(hKey, lpSubKey, ulOptions, samDesired, phkResult);

	COMMONPARAMSVECTOR cpv;
	CCommonParams cp;
	cp.type = ET_ARGS_PCHAR;
	cp.data = "RegOpenKeyExW";
	cpv.push_back(cp);
	cp.type = ET_ARGS_PCHAR;
	cp.data = WToA(lpSubKey);
	cpv.push_back(cp);
	log(&cpv);

	return lResult;
}

typedef LONG WINAPI TYPE_RegQueryValueExW(
	_In_        HKEY    hKey,
	_In_opt_    LPCWSTR lpValueName,
	_Reserved_  LPDWORD lpReserved,
	_Out_opt_   LPDWORD lpType,
	_Out_opt_   LPBYTE  lpData,
	_Inout_opt_ LPDWORD lpcbData
);
FUNC_INIT(ADVAPI32, RegQueryValueExW)
LONG WINAPI HOOK_RegQueryValueExW(
	_In_        HKEY    hKey,
	_In_opt_    LPCWSTR lpValueName,
	_Reserved_  LPDWORD lpReserved,
	_Out_opt_   LPDWORD lpType,
	_Out_opt_   LPBYTE  lpData,
	_Inout_opt_ LPDWORD lpcbData)
{
	LONG lResult = 0;
	
	lResult = TRUE_RegQueryValueExW(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
	
	COMMONPARAMSVECTOR cpv;
	CCommonParams cp;
	cp.type = ET_ARGS_PCHAR;
	cp.data = "RegQueryValueExW";
	cpv.push_back(cp);
	cp.type = ET_ARGS_PCHAR;
	cp.data = WToA(lpValueName);
	cpv.push_back(cp);
	DWORD dwType = 0;
	SIZE_T stSize = 0;
	ReadProcessMemoryValue(GetCurrentProcess(), lpType, &dwType, sizeof(dwType), &stSize);
	cp.type = ET_ARGS_PCHAR;
	cp.data = STRING_FROM_HEX(dwType);
	cpv.push_back(cp);
	cp.type = ET_ARGS_PCHAR;
	cp.data = WToA((PWCHAR)lpData);
	cpv.push_back(cp);

	log(&cpv);

	return lResult;
}

typedef HINSTANCE TYPE_ShellExecuteW(
	_In_opt_ HWND    hwnd,
	_In_opt_ LPCWSTR lpOperation,
	_In_     LPCWSTR lpFile,
	_In_opt_ LPCWSTR lpParameters,
	_In_opt_ LPCWSTR lpDirectory,
	_In_     INT     nShowCmd
	);
FUNC_INIT(SHELL32, ShellExecuteW)
HINSTANCE HOOK_ShellExecuteW(
	_In_opt_ HWND    hwnd,
	_In_opt_ LPCWSTR lpOperation,
	_In_     LPCWSTR lpFile,
	_In_opt_ LPCWSTR lpParameters,
	_In_opt_ LPCWSTR lpDirectory,
	_In_     INT     nShowCmd
)
{
	HINSTANCE hInstance = NULL;

	COMMONPARAMSVECTOR cpv;
	CCommonParams cp;
	cp.type = ET_ARGS_PCHAR;
	cp.data = "ShellExecuteW";
	cpv.push_back(cp);
	cp.type = ET_ARGS_PCHAR;
	cp.data = WToA(lpOperation);
	cpv.push_back(cp);
	cp.type = ET_ARGS_PCHAR;
	cp.data = WToA(lpFile);
	cpv.push_back(cp);
	cp.type = ET_ARGS_PCHAR;
	cp.data = WToA(lpParameters);
	cpv.push_back(cp);
	cp.type = ET_ARGS_PCHAR;
	cp.data = WToA(lpDirectory);
	cpv.push_back(cp);
	log(&cpv);

	hInstance = TRUE_ShellExecuteW(hwnd, lpOperation, lpFile, lpParameters, lpDirectory, nShowCmd);

	return hInstance;
}

typedef HANDLE WINAPI TYPE_CreateFileW(
	_In_     LPCWSTR               lpFileName,
	_In_     DWORD                 dwDesiredAccess,
	_In_     DWORD                 dwShareMode,
	_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	_In_     DWORD                 dwCreationDisposition,
	_In_     DWORD                 dwFlagsAndAttributes,
	_In_opt_ HANDLE                hTemplateFile
	);
FUNC_INIT(KERNEL32, CreateFileW)
HANDLE WINAPI HOOK_CreateFileW(
	_In_     LPCWSTR               lpFileName,
	_In_     DWORD                 dwDesiredAccess,
	_In_     DWORD                 dwShareMode,
	_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	_In_     DWORD                 dwCreationDisposition,
	_In_     DWORD                 dwFlagsAndAttributes,
	_In_opt_ HANDLE                hTemplateFile
)
{
	HANDLE hResult = NULL;
	COMMONPARAMSVECTOR cpv;
	CCommonParams cp;

	cp.type = ET_ARGS_PCHAR;
	cp.data = "CreateFileW";
	cpv.push_back(cp);
	cp.type = ET_ARGS_PCHAR;
	cp.data = WToA(lpFileName);
	cpv.push_back(cp);
	log(&cpv);

	hResult = TRUE_CreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);

	return hResult;
}

typedef BOOL WINAPI TYPE_WriteFile(
	_In_        HANDLE       hFile,
	_In_        LPCVOID      lpBuffer,
	_In_        DWORD        nNumberOfBytesToWrite,
	_Out_opt_   LPDWORD      lpNumberOfBytesWritten,
	_Inout_opt_ LPOVERLAPPED lpOverlapped
);
FUNC_INIT(KERNEL32, WriteFile)
BOOL WINAPI HOOK_WriteFile(
	_In_        HANDLE       hFile,
	_In_        LPCVOID      lpBuffer,
	_In_        DWORD        nNumberOfBytesToWrite,
	_Out_opt_   LPDWORD      lpNumberOfBytesWritten,
	_Inout_opt_ LPOVERLAPPED lpOverlapped
)
{
	BOOL bResult = FALSE;
	COMMONPARAMSVECTOR cpv;
	CCommonParams cp;

	bResult = TRUE_WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);

	cp.type = ET_ARGS_PCHAR;
	cp.data = "WriteFile";
	cpv.push_back(cp);
	cp.type = ET_ARGS_PCHAR;
	cp.data = WToA((PWCHAR)lpBuffer);
	cpv.push_back(cp);
	log(&cpv);

	return bResult;
}

#define INTERNET_OPTION_REFRESH                 37
#define INTERNET_OPTION_PROXY                   38
#define INTERNET_OPTION_SETTINGS_CHANGED        39
typedef LPVOID HINTERNET;
typedef BOOL TYPE_InternetSetOption(
	_In_ HINTERNET hInternet,
	_In_ DWORD     dwOption,
	_In_ LPVOID    lpBuffer,
	_In_ DWORD     dwBufferLength
);
//FUNC_INIT(WININET, InternetSetOption)

#define WINHTTP_ACCESS_TYPE_DEFAULT_PROXY               0
#define WINHTTP_ACCESS_TYPE_NO_PROXY                    1
#define WINHTTP_ACCESS_TYPE_NAMED_PROXY                 3
typedef struct
{
	DWORD  dwAccessType;      // see WINHTTP_ACCESS_* types below
	LPWSTR lpszProxy;         // proxy server list
	LPWSTR lpszProxyBypass;   // proxy bypass list
}
WINHTTP_PROXY_INFO, *LPWINHTTP_PROXY_INFO;
typedef BOOL WINAPI TYPE_WinHttpSetDefaultProxyConfiguration(
	_In_ WINHTTP_PROXY_INFO *pProxyInfo
);
//FUNC_INIT(WINHTTTP, WinHttpSetDefaultProxyConfiguration)
#include <urlmon.h>
#include <wininet.h>
#pragma comment(lib, "urlmon")
typedef int WSAAPI TYPE_send(
	_In_       SOCKET s,
	_In_ const char   *buf,
	_In_       int    len,
	_In_       int    flags
	);
FUNC_INIT(WS2_32, send)
int WSAAPI HOOK_send(
	_In_       SOCKET s,
	_In_ const char   *buf,
	_In_       int    len,
	_In_       int    flags
)
{
	int nResult = 0;
	COMMONPARAMSVECTOR cpv;
	CCommonParams cp;

	printf("0x%x--\n", TRUE_send);
	{

		INTERNET_PROXY_INFO proxy;
		proxy.dwAccessType = INTERNET_OPEN_TYPE_PROXY;
		proxy.lpszProxy = "127.0.0.1:3128";
		proxy.lpszProxyBypass = NULL;

		HRESULT hr = ::UrlMkSetSessionOption(INTERNET_OPTION_PROXY, &proxy, sizeof(proxy), 0);
		//TYPE_WinHttpSetDefaultProxyConfiguration * TRUE_WinHttpSetDefaultProxyConfiguration = (TYPE_WinHttpSetDefaultProxyConfiguration *)FUNC_LOCAL_INIT(_T("WINHTTP"), "WinHttpSetDefaultProxyConfiguration");
		//printf("0x%x--\n", TRUE_WinHttpSetDefaultProxyConfiguration);
		//TYPE_InternetSetOption * TRUE_InternetSetOption = (TYPE_InternetSetOption *)FUNC_LOCAL_INIT(_T("WININET"), "InternetSetOption");
		//printf("0x%x--\n", TRUE_InternetSetOption);

		//WINHTTP_PROXY_INFO proxyinfo = { 0 };
		//proxyinfo.dwAccessType = WINHTTP_ACCESS_TYPE_NAMED_PROXY;
		//proxyinfo.lpszProxy = L"http=127.0.0.1:3128;https=127.0.0.1:3128";
		//proxyinfo.lpszProxyBypass = L"";
		//bResult = ::WinHttpGetDefaultProxyConfiguration(&proxyinfo);
		//printf("0x%x===0x%x===\n", TRUE_WinHttpSetDefaultProxyConfiguration, TRUE_InternetSetOption);
		//TRUE_WinHttpSetDefaultProxyConfiguration(&proxyinfo);
		//通知注册表中代理改变,下次连接时启动代理
		//TRUE_InternetSetOption(NULL, INTERNET_OPTION_SETTINGS_CHANGED, NULL, 0);
		//从注册表中读入代理
		//TRUE_InternetSetOption(NULL, INTERNET_OPTION_REFRESH, NULL, 0);
	}
	nResult = TRUE_send(s, buf, len, flags);

	cp.type = ET_ARGS_PCHAR;
	cp.data = "send";
	cpv.push_back(cp);
	//log(&cpv);

	return nResult;
}
typedef int WSAAPI TYPE_recv(
	_In_       SOCKET s,
	_In_ const char   *buf,
	_In_       int    len,
	_In_       int    flags
	);
FUNC_INIT(WS2_32, recv)
int WSAAPI HOOK_recv(
	_In_       SOCKET s,
	_In_ const char   *buf,
	_In_       int    len,
	_In_       int    flags
)
{
	int nResult = 0;
	COMMONPARAMSVECTOR cpv;
	CCommonParams cp;

	nResult = TRUE_recv(s, buf, len, flags);

	cp.type = ET_ARGS_PCHAR;
	cp.data = "recv";
	cpv.push_back(cp);
	log(&cpv);

	return nResult;
}

typedef struct
{
	BOOL    fAutoDetect;
	LPWSTR  lpszAutoConfigUrl;
	LPWSTR  lpszProxy;
	LPWSTR  lpszProxyBypass;
} WINHTTP_CURRENT_USER_IE_PROXY_CONFIG;
typedef BOOL WINAPI TYPE_WinHttpGetIEProxyConfigForCurrentUser(
	_Inout_ WINHTTP_CURRENT_USER_IE_PROXY_CONFIG *pProxyConfig
	);
FUNC_INIT(WINHTTP, WinHttpGetIEProxyConfigForCurrentUser)
int WSAAPI HOOK_WinHttpGetIEProxyConfigForCurrentUser(
	_Inout_ WINHTTP_CURRENT_USER_IE_PROXY_CONFIG *pProxyConfig
)
{
	int nResult = 0;
	COMMONPARAMSVECTOR cpv;
	CCommonParams cp;

	nResult = TRUE_WinHttpGetIEProxyConfigForCurrentUser(pProxyConfig);

	cp.type = ET_ARGS_PCHAR;
	cp.data = "WinHttpGetIEProxyConfigForCurrentUser";
	cpv.push_back(cp);
	cp.type = ET_ARGS_PCHAR;
	cp.data = pProxyConfig->fAutoDetect ? "True" : "False";
	cpv.push_back(cp);
	cp.type = ET_ARGS_PCHAR;
	cp.data = WToA(pProxyConfig->lpszAutoConfigUrl);
	cpv.push_back(cp);
	cp.type = ET_ARGS_PCHAR;
	cp.data = WToA(pProxyConfig->lpszProxy);
	cpv.push_back(cp);
	cp.type = ET_ARGS_PCHAR;
	cp.data = WToA(pProxyConfig->lpszProxyBypass);
	cpv.push_back(cp);
	log(&cpv);

	return nResult;
}

typedef struct
{
	DWORD   dwFlags;
	DWORD   dwAutoDetectFlags;
	LPCWSTR lpszAutoConfigUrl;
	LPVOID  lpvReserved;
	DWORD   dwReserved;
	BOOL    fAutoLogonIfChallenged;
}
WINHTTP_AUTOPROXY_OPTIONS;
typedef BOOL WINAPI TYPE_WinHttpGetProxyForUrl(
	_In_  HINTERNET                 hSession,
	_In_  LPCWSTR                   lpcwszUrl,
	_In_  WINHTTP_AUTOPROXY_OPTIONS *pAutoProxyOptions,
	_Out_ WINHTTP_PROXY_INFO        *pProxyInfo
);
FUNC_INIT(WINHTTP, WinHttpGetProxyForUrl)
BOOL WINAPI HOOK_WinHttpGetProxyForUrl(
	_In_  HINTERNET                 hSession,
	_In_  LPCWSTR                   lpcwszUrl,
	_In_  WINHTTP_AUTOPROXY_OPTIONS *pAutoProxyOptions,
	_Out_ WINHTTP_PROXY_INFO        *pProxyInfo
)
{
	BOOL bResult = FALSE;
	COMMONPARAMSVECTOR cpv;
	CCommonParams cp;

	bResult = TRUE_WinHttpGetProxyForUrl(hSession, lpcwszUrl, pAutoProxyOptions, pProxyInfo);
	cp.type = ET_ARGS_PCHAR;
	cp.data = "WinHttpGetProxyForUrl";
	cpv.push_back(cp);
	cp.type = ET_ARGS_PCHAR;
	cp.data = WToA(lpcwszUrl);
	cpv.push_back(cp);
	cp.type = ET_ARGS_PCHAR;
	cp.data = WToA((pAutoProxyOptions && pAutoProxyOptions->lpszAutoConfigUrl) ? pAutoProxyOptions->lpszAutoConfigUrl : L"");
	cpv.push_back(cp);
	cp.type = ET_ARGS_PCHAR;
	cp.data = WToA(pProxyInfo->lpszProxy);
	cpv.push_back(cp);
	cp.type = ET_ARGS_PCHAR;
	cp.data = WToA(pProxyInfo->lpszProxyBypass);
	cpv.push_back(cp);
	log(&cpv);

	return bResult;
}

typedef BOOL WINAPI TYPE_CreateProcessW(
	_In_opt_    LPCWSTR               lpApplicationName,
	_Inout_opt_ LPWSTR                lpCommandLine,
	_In_opt_    LPSECURITY_ATTRIBUTES lpProcessAttributes,
	_In_opt_    LPSECURITY_ATTRIBUTES lpThreadAttributes,
	_In_        BOOL                  bInheritHandles,
	_In_        DWORD                 dwCreationFlags,
	_In_opt_    LPVOID                lpEnvironment,
	_In_opt_    LPCWSTR               lpCurrentDirectory,
	_In_        LPSTARTUPINFOW         lpStartupInfo,
	_Out_       LPPROCESS_INFORMATION lpProcessInformation
);
FUNC_INIT(KERNEL32, CreateProcessW)
BOOL WINAPI HOOK_CreateProcessW(
	_In_opt_    LPCWSTR               lpApplicationName,
	_Inout_opt_ LPWSTR                lpCommandLine,
	_In_opt_    LPSECURITY_ATTRIBUTES lpProcessAttributes,
	_In_opt_    LPSECURITY_ATTRIBUTES lpThreadAttributes,
	_In_        BOOL                  bInheritHandles,
	_In_        DWORD                 dwCreationFlags,
	_In_opt_    LPVOID                lpEnvironment,
	_In_opt_    LPCWSTR               lpCurrentDirectory,
	_In_        LPSTARTUPINFOW        lpStartupInfo,
	_Out_       LPPROCESS_INFORMATION lpProcessInformation
)
{
	BOOL bResult = FALSE;
	COMMONPARAMSVECTOR cpv;
	CCommonParams cp;

	bResult = TRUE_CreateProcessW(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
	cp.type = ET_ARGS_PCHAR;
	cp.data = "CreateProcessW";
	cpv.push_back(cp);
	cp.type = ET_ARGS_PCHAR;
	cp.data = WToA(lpApplicationName ? lpApplicationName : L"");
	cpv.push_back(cp);
	cp.type = ET_ARGS_PCHAR;
	cp.data = WToA(lpCommandLine ? lpCommandLine : L"");
	cpv.push_back(cp);
	cp.type = ET_ARGS_PCHAR;
	cp.data = WToA(lpCurrentDirectory);
	cpv.push_back(cp);
	log(&cpv);

	return bResult;
}

typedef HLOCAL WINAPI TYPE_LocalAlloc(
	_In_ UINT   uFlags,
	_In_ SIZE_T uBytes
);
FUNC_INIT(KERNEL32, LocalAlloc)
HLOCAL WINAPI HOOK_LocalAlloc(
	_In_ UINT   uFlags,
	_In_ SIZE_T uBytes
)
{
	HLOCAL hResult = 0;
	COMMONPARAMSVECTOR cpv;
	CCommonParams cp;

	hResult = TRUE_LocalAlloc(uFlags, uBytes);
	cp.type = ET_ARGS_PCHAR;
	cp.data = "LocalAlloc";
	cpv.push_back(cp);
	cp.type = ET_ARGS_PCHAR;
	cp.data = STRING_FROM_HEX(uFlags);
	cpv.push_back(cp);
	cp.type = ET_ARGS_PCHAR;
	cp.data = STRING_FROM_HEX(uBytes);
	cpv.push_back(cp);
	cp.type = ET_ARGS_PCHAR;
	cp.data = STRING_FROM_HEX((unsigned long)hResult);
	cpv.push_back(cp);
	log(&cpv);

	return hResult;
}

typedef HGLOBAL WINAPI TYPE_GlobalAlloc(
	_In_ UINT   uFlags,
	_In_ SIZE_T dwBytes
);
FUNC_INIT(KERNEL32, GlobalAlloc)
HGLOBAL WINAPI HOOK_GlobalAlloc(
	_In_ UINT   uFlags,
	_In_ SIZE_T dwBytes
)
{
	HGLOBAL hResult = 0;
	COMMONPARAMSVECTOR cpv;
	CCommonParams cp;

	hResult = TRUE_LocalAlloc(uFlags, dwBytes);
	cp.type = ET_ARGS_PCHAR;
	cp.data = "GlobalAlloc";
	cpv.push_back(cp);
	cp.type = ET_ARGS_PCHAR;
	cp.data = STRING_FROM_HEX(uFlags);
	cpv.push_back(cp);
	cp.type = ET_ARGS_PCHAR;
	cp.data = STRING_FROM_HEX(dwBytes);
	cpv.push_back(cp);
	cp.type = ET_ARGS_PCHAR;
	cp.data = STRING_FROM_HEX((unsigned long)hResult);
	cpv.push_back(cp);
	log(&cpv);

	return hResult;
}

typedef BOOL WINAPI TYPE_SetWindowTextW(
	_In_     HWND    hWnd,
	_In_opt_ LPCWSTR lpString
);
FUNC_INIT(USER32, SetWindowTextW)
BOOL WINAPI HOOK_SetWindowTextW(
	_In_     HWND    hWnd,
	_In_opt_ LPCWSTR lpString
)
{
	BOOL bResult = FALSE;
	COMMONPARAMSVECTOR cpv;
	CCommonParams cp;

	cp.type = ET_ARGS_PCHAR;
	cp.data = "SetWindowTextW";
	cpv.push_back(cp);
	cp.type = ET_ARGS_PCHAR;
	cp.data = STRING_FROM_HEX((unsigned long)hWnd);
	cpv.push_back(cp);
	cp.type = ET_ARGS_PCHAR;
	cp.data = WToA(lpString);
	cpv.push_back(cp);
	log(&cpv);

	bResult = TRUE_SetWindowTextW(hWnd, lpString);

	return bResult;
}

typedef int WINAPI TYPE_GetWindowTextW(
	_In_  HWND   hWnd,
	_Out_ LPWSTR lpString,
	_In_  int    nMaxCount
);
FUNC_INIT(USER32, GetWindowTextW)
int WINAPI HOOK_GetWindowTextW(
	_In_  HWND   hWnd,
	_Out_ LPWSTR lpString,
	_In_  int    nMaxCount
)
{
	int nResult = 0;
	COMMONPARAMSVECTOR cpv;
	CCommonParams cp;

	nResult = TRUE_GetWindowTextW(hWnd, lpString, nMaxCount);

	cp.type = ET_ARGS_PCHAR;
	cp.data = "GetWindowTextW";
	cpv.push_back(cp);
	cp.type = ET_ARGS_PCHAR;
	cp.data = STRING_FROM_HEX((unsigned long)hWnd);
	cpv.push_back(cp);
	cp.type = ET_ARGS_PCHAR;
	cp.data = WToA(lpString);
	cpv.push_back(cp);
	cp.type = ET_ARGS_PCHAR;
	cp.data = STRING_FROM_HEX(nMaxCount);
	cpv.push_back(cp);
	cp.type = ET_ARGS_PCHAR;
	cp.data = STRING_FROM_HEX(nResult);
	cpv.push_back(cp);
	log(&cpv);

	return nResult;
}

typedef LRESULT WINAPI TYPE_SendMessageW(
	_In_ HWND   hWnd,
	_In_ UINT   Msg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
);
FUNC_INIT(USER32, SendMessageW)
LRESULT WINAPI HOOK_SendMessageW(
	_In_ HWND   hWnd,
	_In_ UINT   Msg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
)
{
	LRESULT lResult = 0;
	COMMONPARAMSVECTOR cpv;
	CCommonParams cp;

	lResult = TRUE_SendMessageW(hWnd, Msg, wParam, lParam);

	cp.type = ET_ARGS_PCHAR;
	cp.data = "SendMessageW";
	cpv.push_back(cp);
	cp.type = ET_ARGS_PCHAR;
	cp.data = STRING_FROM_HEX((unsigned long)hWnd);
	cpv.push_back(cp);
	cp.type = ET_ARGS_PCHAR;
	cp.data = STRING_FROM_HEX(Msg);
	cpv.push_back(cp);
	cp.type = ET_ARGS_PCHAR;
	cp.data = STRING_FROM_HEX(wParam);
	cpv.push_back(cp);
	cp.type = ET_ARGS_PCHAR;
	cp.data = STRING_FROM_HEX(lParam);
	cpv.push_back(cp);
	cp.type = ET_ARGS_PCHAR;
	cp.data = STRING_FROM_HEX(lResult);
	cpv.push_back(cp);
	log(&cpv);

	return lResult;
}

typedef BOOL WINAPI TYPE_PostMessageW(
	_In_opt_ HWND   hWnd,
	_In_     UINT   Msg,
	_In_     WPARAM wParam,
	_In_     LPARAM lParam
);
FUNC_INIT(USER32, PostMessageW)
BOOL WINAPI HOOK_PostMessageW(
	_In_opt_ HWND   hWnd,
	_In_     UINT   Msg,
	_In_     WPARAM wParam,
	_In_     LPARAM lParam
)
{
	BOOL bResult = 0;
	COMMONPARAMSVECTOR cpv;
	CCommonParams cp;

	bResult = TRUE_PostMessageW(hWnd, Msg, wParam, lParam);
	
	cp.type = ET_ARGS_PCHAR;
	cp.data = "PostMessageW";
	cpv.push_back(cp);
	cp.type = ET_ARGS_PCHAR;
	cp.data = STRING_FROM_HEX((unsigned long)hWnd);
	cpv.push_back(cp);
	cp.type = ET_ARGS_PCHAR;
	cp.data = STRING_FROM_HEX(Msg);
	cpv.push_back(cp);
	cp.type = ET_ARGS_PCHAR;
	cp.data = STRING_FROM_HEX(wParam);
	cpv.push_back(cp);
	cp.type = ET_ARGS_PCHAR;
	cp.data = STRING_FROM_HEX(lParam);
	cpv.push_back(cp);
	cp.type = ET_ARGS_PCHAR;
	cp.data = STRING_FROM_HEX(bResult);
	cpv.push_back(cp);
	log(&cpv);

	return bResult;
}
