// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once
#ifndef __STDAFX_H_
#define __STDAFX_H_

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <winsock2.h>
#include <stdio.h>
#include <tchar.h>

#include <string>
#include <vector>

// TODO: reference additional headers your program requires here
#include "mhook-lib/mhook.h"

#if !defined(UNICODE) && !defined(_UNICODE)
#define tstring std::string
#else
#define tstring std::wstring
#endif

#define MAX_BASE_SIZE   128
#define MAX_NAME_SIZE   256
#define MAX_CMDS_SIZE   1024
#define MAX_DATA_SIZE   16384

#define LOGG_FILE_NAME	"file.log"

static int G_nFD = 0;
static long G_ltv_sec = 0;

//整型转换为字符串类型
static __inline tstring STRING_FROM_INT(int v) { _TCHAR tzV[MAX_BASE_SIZE] = { 0 }; memset(tzV, 0, sizeof(tzV)); _sntprintf(tzV, sizeof(tzV) / sizeof(_TCHAR), _T("%d"), v); return tstring(tzV); }
static __inline tstring STRING_FROM_LONG(long v) { _TCHAR tzV[MAX_BASE_SIZE] = { 0 }; memset(tzV, 0, sizeof(tzV)); _sntprintf(tzV, sizeof(tzV) / sizeof(_TCHAR), _T("%ld"), v); return tstring(tzV); }
static __inline tstring STRING_FROM_FLOART(float v) { _TCHAR tzV[MAX_BASE_SIZE] = { 0 }; memset(tzV, 0, sizeof(tzV)); _sntprintf(tzV, sizeof(tzV) / sizeof(_TCHAR), _T("%lf"), v); return tstring(tzV); }
static __inline tstring STRING_FROM_HEX(unsigned long v) { _TCHAR tzV[MAX_BASE_SIZE] = { 0 }; memset(tzV, 0, sizeof(tzV)); _sntprintf(tzV, sizeof(tzV) / sizeof(_TCHAR), _T("0x%x"), v); return tstring(tzV); }

#include <time.h>

//根据秒时间获取日期
static __inline tstring DATE_FROM_TIME(time_t tv_sec) { _TCHAR tzV[MAX_BASE_SIZE] = { 0 }; struct tm * tm = localtime(&tv_sec); memset(tzV, 0, sizeof(tzV)); _tcsftime(tzV, sizeof(tzV) / sizeof(_TCHAR), _T("%Y%m%d"), tm); return tstring(tzV); }
//根据秒时间获取精确微秒时间
static __inline tstring STRING_FROM_TIME(struct timeval * ptv) { time_t tt = ptv->tv_sec; struct tm * tm = localtime((const time_t *)&tt); _TCHAR tzV[MAX_BASE_SIZE] = { 0 }; memset(tzV, 0, sizeof(tzV)); _tcsftime(tzV, sizeof(tzV) / sizeof(_TCHAR), _T("%Y-%m-%d %H:%M:%S"), tm); return tstring(tzV) + _T(".") + STRING_FROM_LONG(ptv->tv_usec); }
#include <log4z.h>
using namespace zsummer::log4z;

#include <fcntl.h>
#ifndef fsync
#include <io.h>
#define fsync _commit
#endif // !fsync
#ifndef gettimeofday
static int gettimeofday(struct timeval* tv)
{
	typedef union {		
		FILETIME filetime;
		unsigned long long nanotime;
	} NANOTIME;
	NANOTIME nt = { 0 };
	::GetSystemTimeAsFileTime(&nt.filetime);
	tv->tv_usec = (long)((nt.nanotime / 10ULL) % 1000000ULL);
	tv->tv_sec = (long)((nt.nanotime - 116444736000000000ULL) / 10000000ULL);

	return (0);
}
#endif // !gettimeofday


__inline static std::string TToA(tstring tsT);
__inline static std::wstring TToW(tstring tsT);
__inline static tstring AToT(std::string str);
__inline static tstring WToT(std::wstring wstr);

//记录日志接口
static __inline void DebugPrint(int fd, const void * data, unsigned long size)
{
	write(fd, data, size);
	//fsync(fd);
}

//记录日志接口
static __inline void DebugPrint(int fd, const _TCHAR * data, struct timeval * tv)
{
	//printf("fd=%d\n\n", fd);
	DebugPrint(fd, TToA(data).c_str(), TToA(data).length());
	std::string strLog = TToA(tstring(_T("[")) + STRING_FROM_TIME(tv) + _T("] ") + data + _T("\n"));
	//printf("%s\n", strLog.c_str());
	DebugPrint(fd, strLog.c_str(), strLog.length());
}

//记录日志接口
static __inline void DebugPrint(int & fd, const _TCHAR * logfilename, const _TCHAR * pLogInfo)
{
	struct  timeval  tv = { 0 };
	gettimeofday(&tv);
	if (fd <= 0)
	{
		//不是今天
		if (DATE_FROM_TIME(G_ltv_sec).compare(DATE_FROM_TIME(tv.tv_sec)))
		{
			G_ltv_sec = tv.tv_sec;

			//fd = _topen(logfilename, O_CREAT | O_TRUNC | O_RDWR);
			fd = _topen(logfilename, O_CREAT | O_TRUNC | O_RDWR, 0777);
		}

		if (fd <= 0)
		{
			//fd = _topen(logfilename, O_CREAT | O_APPEND | O_RDWR);
			fd = _topen(logfilename, O_APPEND | O_RDWR, 0777);
		}
	}
	if (fd > 0)
	{
		DebugPrint(fd, pLogInfo, &tv);
	}
}
//记录日志接口
static __inline void DebugPrintC(const _TCHAR * pLogInfo)
{
	DebugPrint(G_nFD, _T(LOGG_FILE_NAME), pLogInfo);
}
//记录日志接口
static __inline void DebugPrintString(tstring tsLogInfo)
{
	//LOGD(tsLogInfo.c_str());
	//printf("%s\n", TToA(tsLogInfo).c_str());
	DebugPrintC(tsLogInfo.c_str());
}

//获取程序文件路径
static __inline tstring GetProgramPath(HINSTANCE hInstance = NULL)
{
	tstring tstrFilePath = _T("");
	_TCHAR tFilePath[MAX_PATH] = { 0 };
	GetModuleFileName(hInstance, tFilePath, MAX_PATH);
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

//	ANSI to Unicode
__inline static std::wstring ANSIToUnicode(const std::string& str)
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
__inline static std::string UnicodeToANSI(const std::wstring& str)
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
__inline static std::wstring UTF8ToUnicode(const std::string& str)
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
__inline static std::string UnicodeToUTF8(const std::wstring& str)
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

#endif // !__STDAFX_H_