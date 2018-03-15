// HookProcDemo.cpp : Defines the entry point for the console application.
//

#include "header.h"
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
void Usage(_TCHAR * ptText)
{
	_tprintf(_T("Usage:\n\t%s [hook module] [process name]\n"), ptText);
}
#include <time.h>
//根据秒时间获取日期
static __inline tstring DATE_FROM_TIME(time_t tv_sec) { _TCHAR tzV[128] = { 0 }; struct tm * tm = localtime(&tv_sec); memset(tzV, 0, sizeof(tzV)); _tcsftime(tzV, sizeof(tzV) / sizeof(_TCHAR), _T("%Y%m%d"), tm); return tstring(tzV); }

int _tmain(int argc, _TCHAR* argv[])
{
	int n = CONTEXT_ALL;
	struct timeval tv = { 0 };
	gettimeofday(&tv);
	DATE_FROM_TIME(0);
	DATE_FROM_TIME(tv.tv_sec);
	_TCHAR tszModuleName[MAX_PATH] = _T("hookproc.dll");
	_TCHAR tszProcessName[MAX_PATH] = _T("D:\\DevelopmentEnvironment\\Softwares\\Tencent\\WeChat\\wechat.exe");
	switch (argc)
	{
	case 1:
	{
	}
	break;
	case 3:
	{
		memset(tszModuleName, 0, sizeof(tszModuleName));
		memset(tszProcessName, 0, sizeof(tszProcessName));
		_tcscpy(tszModuleName, argv[1]);
		_tcscpy(tszProcessName, argv[2]);
	}
	break;
	default:
	{
		Usage(argv[0]);
		return 0;
	}
		break;
	}
	tstring tsDllPath = GetProgramPath() + tszModuleName;
	
	PROCESS_INFORMATION pi = { 0 };
	StartupProgram(tszProcessName, _T(""), NULL, &pi);
	InjectDll(pi.dwProcessId, pi.dwThreadId, tsDllPath.c_str());

	//BOOL isInject = InjectDllToRemoteProcess(tsDllPath.c_str(), NULL, tszProcessName);

	//if (!isInject)
	{
		//注入远程进程失败
		//::MessageBox(NULL, _T("Remote inject error"), _T("error"), MB_OK);
	}

	while (1)
	{
		Sleep(1000);
		
		_tprintf(_T("Handling...\n"));

		Sleep(5000);
		
		break;
	}

	_tprintf(_T("Exiting...\n"));
	
	return 0;
}
