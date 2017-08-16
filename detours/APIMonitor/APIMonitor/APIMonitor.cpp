// APIMonitor.cpp : Defines the entry point for the console application.
//


#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#define EOL "\n"
#define DLL "APIMonitorDriver.dll"
#define BANNER "WinAPI _ Monitoring ~ @ppshuai" EOL
#define USAGE "USAGE: APIMonitor.exe <file.exe>" EOL

//int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nShowCmd)
int _tmain(int argc, _TCHAR* argv[])
{
	int result = 0;
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	if (__argc != 2) {
		printf(BANNER);
		printf(USAGE);
		exit(EXIT_FAILURE);
	}
	HWND hWnd = ::FindWindowEx(NULL, NULL, _T("CefWebViewWnd"), NULL);
	if (hWnd)
	{
		//GetWindowThreadProcessId(hWnd, &pi.dwProcessId);
	}
	if (!CreateProcess(__targv[1], NULL, NULL, NULL, FALSE, CREATE_SUSPENDED | NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi)) {
		printf("CreateProcess failed (%d)\n", GetLastError());
		exit(EXIT_FAILURE);
	}

	LPVOID addrAlloc = VirtualAllocEx(pi.hProcess, NULL, strlen(DLL), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (!WriteProcessMemory(pi.hProcess, addrAlloc, DLL, strlen(DLL), NULL)) {
		printf("WriteProcessMemory failed (%d)\n", GetLastError());
		exit(EXIT_FAILURE);
	}

	LPTHREAD_START_ROUTINE addrLoadLibrary = (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(_T("kernel32.dll")), "LoadLibraryA");
	HANDLE RemoteFunction = NULL;
	if (!(RemoteFunction = CreateRemoteThread(pi.hProcess, NULL, 0, addrLoadLibrary, addrAlloc, 0, NULL))) {
		printf("CreateRemoteThread failed (%d)\n", GetLastError());
		exit(EXIT_FAILURE);
	}

	printf("[+] Start monitoring ..." EOL);

	ResumeThread(pi.hThread);
	WaitForSingleObject(RemoteFunction, INFINITE);
	VirtualFreeEx(pi.hProcess, addrAlloc, 0, MEM_DECOMMIT);
	CloseHandle(RemoteFunction);
	WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	return result;
}

