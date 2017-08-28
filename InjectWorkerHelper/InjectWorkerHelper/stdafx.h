// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//  are changed infrequently
//

#pragma once

// Change these values to use different versions
#define WINVER		0x0500
#define _WIN32_WINNT	0x0501
#define _WIN32_IE	0x0501
#define _RICHEDIT_VER	0x0500

#define _WTL_NO_CSTRING
#define _WTL_NO_WTYPES
#define _CSTRING_NS
#define _WTYPES_NS

#include <atlstr.h>
#include <atlbase.h>
#include <atlapp.h>

extern CAppModule _Module;

#include <atlwin.h>

#include <atlframe.h>
#include <atlctrls.h>
#include <atlctrlx.h>
#include <atldlgs.h>
#include <atlddx.h>
#include <atlcrack.h>
#include <atlctrlw.h>
#include <atlmisc.h>
#include <atltypes.h>

#include <windows.h>
#include <tlhelp32.h>

#include <map>
#include <vector>
#include <string>

#if !defined(_UNICODE) && !defined(UNICODE)
#define _TSTRING	std::string
#else
#define _TSTRING	std::wstring
#endif //!defined(_UNICODE) && !defined(UNICODE)

#define TSTRING _TSTRING
#define tstring _TSTRING

__inline static tstring AToT(std::string str);

__inline static tstring WToT(std::wstring wstr);

typedef std::map<std::string, std::string> STRINGSTRINGMAP;
typedef STRINGSTRINGMAP::iterator STRINGSTRINGMAPIT;
typedef STRINGSTRINGMAP::value_type STRINGSTRINGMAPPAIR;

////////////////////////////////////////////////////////////////////

//777A01D5    68 78563412     push    12345678
//777A01DA    9C              pushfd
//777A01DB    60              pushad

//777A01EB    61              popad
//777A01EC    9D              popfd
//777A01ED    C3              retn
//结构必须字节对齐1
#pragma pack(1)  
typedef struct _THREAD_INJECT_CODE
{
	BYTE  byPUSH;
	DWORD dwPUSH_VALUE;
	BYTE  byPUSHFD;
	BYTE  byPUSHAD;
	BYTE  byMOV_EAX;          //mov eax, addr szDllpath
	DWORD dwMOV_EAX_VALUE;
	BYTE  byPUSH_EAX;         //push eax
	BYTE  byMOV_ECX;          //mov ecx, LoadLibrary
	DWORD dwMOV_ECX_VALUE;
	WORD  wCALL_ECX;          //call ecx
	BYTE  byPOPAD;
	BYTE  byPOPFD;
	BYTE  byRETN;
	CHAR  szDllPath[MAX_PATH];
}THREAD_INJECT_CODE, *PTHREAD_INJECT_CODE;
#pragma pack()

//结构必须字节对齐1
#pragma pack(1)  
typedef struct _PROCESS_INJECT_CODE
{
	BYTE  byPUSH;
	DWORD dwPUSH_VALUE;
	BYTE  byPUSHFD;
	BYTE  byPUSHAD;
	BYTE  byMOV_EAX;          //mov eax, addr szDllpath
	DWORD dwMOV_EAX_VALUE;
	BYTE  byPUSH_EAX;         //push eax
	BYTE  byMOV_ECX;          //mov ecx, LoadLibrary
	DWORD dwMOV_ECX_VALUE;
	WORD  wCALL_ECX;          //call ecx
	BYTE  byPOPAD;
	BYTE  byPOPFD;
	BYTE  byRETN;
	CHAR  szDllPath[MAX_PATH];
}PROCESS_INJECT_CODE, *PPROCESS_INJECT_CODE;
#pragma pack()  

//结构必须字节对齐!  
#pragma pack(1)  
typedef struct _DEBUGGEE_INJECT_CODE
{
	BYTE  byMOV_EAX;          //mov eax, addr szDllpath
	DWORD dwMOV_EAX_VALUE;
	BYTE  byPUSH_EAX;         //push eax
	BYTE  byMOV_ECX;          //mov ecx, LoadLibrary
	DWORD dwMOV_ECX_VALUE;
	WORD  wCALL_ECX;          //call ecx
	BYTE  byINT3;             //int 3
	CHAR  szDllPath[MAX_PATH];
}DEBUGGEE_INJECT_CODE, *PDEBUGGEE_INJECT_CODE;
#pragma pack()

////////////////////////////////////////////////////////////////////


//初始化调试窗口显示
__inline static void InitDebugConsole()
{
	FILE *pStdOut = stdout;
	FILE *pStdIn = stdin;
	FILE *pStdErr = stderr;

	if (!AllocConsole())
	{
		_TCHAR tErrorInfos[16384] = { 0 };
		_sntprintf(tErrorInfos, sizeof(tErrorInfos) / sizeof(_TCHAR), _T("控制台生成失败! 错误代码:0x%X。"), GetLastError());
		MessageBox(NULL, tErrorInfos, _T("错误提示"), 0);
		return;
	}
	SetConsoleTitle(_T("TraceDebugWindow"));

	pStdOut = _tfreopen(_T("CONOUT$"), _T("w"), stdout);
	pStdIn = _tfreopen(_T("CONIN$"), _T("r"), stdin);
	pStdErr = _tfreopen(_T("CONERR$"), _T("w"), stderr);
	_tsetlocale(LC_ALL, _T("chs"));
}

//释放掉调试窗口显示
__inline static void ExitDebugConsole()
{
	FreeConsole();
}

//	ANSI to Unicode
__inline static std::wstring ANSIToUnicode(const std::string str)
{
	int len = 0;
	len = str.length();
	int unicodeLen = ::MultiByteToWideChar(CP_ACP,
		0,
		str.c_str(),
		-1,
		NULL,
		0);
	wchar_t * pUnicode;
	pUnicode = new  wchar_t[(unicodeLen + 1)];
	memset(pUnicode, 0, (unicodeLen + 1) * sizeof(wchar_t));
	::MultiByteToWideChar(CP_ACP,
		0,
		str.c_str(),
		-1,
		(LPWSTR)pUnicode,
		unicodeLen);
	std::wstring rt;
	rt = (wchar_t*)pUnicode;
	delete pUnicode;
	return rt;
}

//Unicode to ANSI
__inline static std::string UnicodeToANSI(const std::wstring str)
{
	char* pElementText;
	int iTextLen;
	iTextLen = WideCharToMultiByte(CP_ACP,
		0,
		str.c_str(),
		-1,
		NULL,
		0,
		NULL,
		NULL);
	pElementText = new char[iTextLen + 1];
	memset((void*)pElementText, 0, sizeof(char) * (iTextLen + 1));
	::WideCharToMultiByte(CP_ACP,
		0,
		str.c_str(),
		-1,
		pElementText,
		iTextLen,
		NULL,
		NULL);
	std::string strText;
	strText = pElementText;
	delete[] pElementText;
	return strText;
}
//UTF - 8 to Unicode
__inline static std::wstring UTF8ToUnicode(const std::string str)
{
	int len = 0;
	len = str.length();
	int unicodeLen = ::MultiByteToWideChar(CP_UTF8,
		0,
		str.c_str(),
		-1,
		NULL,
		0);
	wchar_t * pUnicode;
	pUnicode = new wchar_t[unicodeLen + 1];
	memset(pUnicode, 0, (unicodeLen + 1) * sizeof(wchar_t));
	::MultiByteToWideChar(CP_UTF8,
		0,
		str.c_str(),
		-1,
		(LPWSTR)pUnicode,
		unicodeLen);
	std::wstring rt;
	rt = (wchar_t*)pUnicode;
	delete pUnicode;
	return rt;
}
//Unicode to UTF - 8
__inline static std::string UnicodeToUTF8(const std::wstring str)
{
	char*   pElementText;
	int iTextLen;
	iTextLen = WideCharToMultiByte(CP_UTF8,
		0,
		str.c_str(),
		-1,
		NULL,
		0,
		NULL,
		NULL);
	pElementText = new char[iTextLen + 1];
	memset((void*)pElementText, 0, sizeof(char) * (iTextLen + 1));
	::WideCharToMultiByte(CP_UTF8,
		0,
		str.c_str(),
		-1,
		pElementText,
		iTextLen,
		NULL,
		NULL);
	std::string strText;
	strText = pElementText;
	delete[] pElementText;
	return strText;
}

__inline static std::string TToA(tstring tsT)
{
	std::string str = "";

#if !defined(UNICODE) && !defined(_UNICODE)
	str = tsT;
#else
	str = UnicodeToANSI(tsT);
#endif

	return str;
}

__inline static std::wstring TToW(tstring tsT)
{
	std::wstring wstr = L"";

#if !defined(UNICODE) && !defined(_UNICODE)
	wstr = ANSIToUnicode(tsT);
#else
	wstr = tsT;
#endif

	return wstr;
}

__inline static tstring AToT(std::string str)
{
	tstring ts = _T("");

#if !defined(UNICODE) && !defined(_UNICODE)
	ts = str;
#else
	ts = ANSIToUnicode(str);
#endif

	return ts;
}

__inline static tstring WToT(std::wstring wstr)
{
	tstring ts = _T("");

#if !defined(UNICODE) && !defined(_UNICODE)
	ts = UnicodeToANSI(wstr);
#else
	ts = wstr;
#endif

	return ts;
}

//utf8 转 Unicode
__inline static std::wstring Utf82Unicode(const std::string& utf8string)
{
	int widesize = ::MultiByteToWideChar(CP_UTF8, 0, utf8string.c_str(), -1, NULL, 0);
	if (widesize == ERROR_NO_UNICODE_TRANSLATION || widesize == 0)
	{
		return std::wstring(L"");
	}

	std::vector<wchar_t> resultstring(widesize);

	int convresult = ::MultiByteToWideChar(CP_UTF8, 0, utf8string.c_str(), -1, &resultstring[0], widesize);

	if (convresult != widesize)
	{
		return std::wstring(L"");
	}

	return std::wstring(&resultstring[0]);
}

//unicode 转为 ascii
__inline static std::string WideByte2Acsi(std::wstring& wstrcode)
{
	int asciisize = ::WideCharToMultiByte(CP_OEMCP, 0, wstrcode.c_str(), -1, NULL, 0, NULL, NULL);
	if (asciisize == ERROR_NO_UNICODE_TRANSLATION || asciisize == 0)
	{
		return std::string("");
	}
	std::vector<char> resultstring(asciisize);
	int convresult = ::WideCharToMultiByte(CP_OEMCP, 0, wstrcode.c_str(), -1, &resultstring[0], asciisize, NULL, NULL);

	if (convresult != asciisize)
	{
		return std::string("");
	}

	return std::string(&resultstring[0]);
}

//utf-8 转 ascii
__inline static std::string UTF_82ASCII(std::string& strUtf8Code)
{
	std::string strRet("");
	//先把 utf8 转为 unicode
	std::wstring wstr = Utf82Unicode(strUtf8Code);
	//最后把 unicode 转为 ascii
	strRet = WideByte2Acsi(wstr);
	return strRet;
}

///////////////////////////////////////////////////////////////////////


//ascii 转 Unicode
__inline static std::wstring Acsi2WideByte(std::string& strascii)
{
	int widesize = MultiByteToWideChar(CP_ACP, 0, (char*)strascii.c_str(), -1, NULL, 0);
	if (widesize == ERROR_NO_UNICODE_TRANSLATION || widesize == 0)
	{
		return std::wstring(L"");
	}
	std::vector<wchar_t> resultstring(widesize);
	int convresult = MultiByteToWideChar(CP_ACP, 0, (char*)strascii.c_str(), -1, &resultstring[0], widesize);


	if (convresult != widesize)
	{
		return std::wstring(L"");
	}

	return std::wstring(&resultstring[0]);
}


//Unicode 转 Utf8
__inline static std::string Unicode2Utf8(const std::wstring& widestring)
{
	int utf8size = ::WideCharToMultiByte(CP_UTF8, 0, widestring.c_str(), -1, NULL, 0, NULL, NULL);
	if (utf8size == 0)
	{
		return std::string("");
	}

	std::vector<char> resultstring(utf8size);

	int convresult = ::WideCharToMultiByte(CP_UTF8, 0, widestring.c_str(), -1, &resultstring[0], utf8size, NULL, NULL);

	if (convresult != utf8size)
	{
		return std::string("");
	}

	return std::string(&resultstring[0]);
}

//ascii 转 Utf8
__inline static std::string ASCII2UTF_8(std::string& strAsciiCode)
{
	std::string strRet("");
	//先把 ascii 转为 unicode
	std::wstring wstr = Acsi2WideByte(strAsciiCode);
	//最后把 unicode 转为 utf8
	strRet = Unicode2Utf8(wstr);
	return strRet;
}

#if defined _M_IX86
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
