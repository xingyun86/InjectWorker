// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

#include <stdio.h>
#include <stdlib.h>
#include <imagehlp.h>
#pragma comment(lib, "dbghelp")

HANDLE g_hModule = INVALID_HANDLE_VALUE;
PROC g_OriginalCreateFileW;
PROC g_OriginalCopyFileW;

typedef HANDLE WINAPI TYPE_CreateFileW(
	_In_     LPCWSTR               lpFileName,
	_In_     DWORD                 dwDesiredAccess,
	_In_     DWORD                 dwShareMode,
	_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	_In_     DWORD                 dwCreationDisposition,
	_In_     DWORD                 dwFlagsAndAttributes,
	_In_opt_ HANDLE                hTemplateFile
	);
HANDLE WINAPI HookCreateFileW(
	_In_     LPCWSTR               lpFileName,
	_In_     DWORD                 dwDesiredAccess,
	_In_     DWORD                 dwShareMode,
	_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	_In_     DWORD                 dwCreationDisposition,
	_In_     DWORD                 dwFlagsAndAttributes,
	_In_opt_ HANDLE                hTemplateFile
)
{
	HANDLE hReturnValue = NULL;

	TYPE_CreateFileW* fn = (TYPE_CreateFileW*)g_OriginalCreateFileW;
	printf("open file = %s\n", lpFileName);
	hReturnValue = (*fn)(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);

	return hReturnValue;
}


typedef BOOL WINAPI TYPE_CopyFileW(
	LPCWSTR lpExistingFileName,
	LPCWSTR lpNewFileName,
	BOOL bFailIfExists
);

BOOL WINAPI HookCopyFileW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, BOOL bFailIfExists)
{
	BOOL ReturnValue;

	TYPE_CopyFileW* fn = (TYPE_CopyFileW*)g_OriginalCopyFileW;

	ReturnValue = (*fn)(lpExistingFileName, lpNewFileName, bFailIfExists);

	return ReturnValue;
}

void SetHook(HMODULE hModuleOfCaller, LPSTR LibraryName, PROC OldFunctionPointer, PROC NewFunctionPointer)
{
	if (hModuleOfCaller == g_hModule)
		return;
	if (hModuleOfCaller == 0)
		return;

	ULONG ulSize;

	// Get the address of the module抯 import section
	PIMAGE_IMPORT_DESCRIPTOR pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)ImageDirectoryEntryToData
	(
		hModuleOfCaller,
		TRUE,
		IMAGE_DIRECTORY_ENTRY_IMPORT,
		&ulSize
	);

	// Does this module have an import section ?
	if (pImportDesc == NULL)
		return;

	// Loop through all descriptors and find the 
	// import descriptor containing references to callee抯 functions
	while (pImportDesc->Name)
	{
		PSTR pszModName = (PSTR)((PBYTE)hModuleOfCaller + pImportDesc->Name);

		if (stricmp(pszModName, LibraryName) == 0)
			break; // Found

		pImportDesc++;
	} // while

	if (pImportDesc->Name == 0)
		return;

	//Get caller IAT 
	PIMAGE_THUNK_DATA pThunk = (PIMAGE_THUNK_DATA)((PBYTE)hModuleOfCaller + pImportDesc->FirstThunk);

	PROC pfnCurrent = OldFunctionPointer;

	// Replace current function address with new one
	while (pThunk->u1.Function)
	{
		// Get the address of the function address
		PROC* ppfn = (PROC*)&pThunk->u1.Function;
		// Is this the function we抮e looking for?
		BOOL bFound = (*ppfn == pfnCurrent);

		if (bFound)
		{
			MEMORY_BASIC_INFORMATION mbi;

			::VirtualQuery(ppfn, &mbi, sizeof(MEMORY_BASIC_INFORMATION));

			// In order to provide writable access to this part of the 
			// memory we need to change the memory protection

			if (FALSE == ::VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_READWRITE, &mbi.Protect))
				return;

			*ppfn = *NewFunctionPointer;

			BOOL bResult = TRUE;

			// Restore the protection back
			DWORD dwOldProtect;

			::VirtualProtect(mbi.BaseAddress, mbi.RegionSize, mbi.Protect, &dwOldProtect);

			break;
		} // if

		pThunk++;

	} // while
}

PROC EnumAndSetHooks(LPSTR BaseLibraryName, LPSTR BaseFunctionName, PROC NewFunctionPointer, bool UnHook, PROC Custom)
{
	HMODULE hMods[1024];
	DWORD cbNeeded;
	unsigned int i;
	typedef BOOL(WINAPI * PFNENUMPROCESSMODULES)
		(
			HANDLE hProcess,
			HMODULE *lphModule,
			DWORD cb,
			LPDWORD lpcbNeeded
			);

	HMODULE hBaseLib = LoadLibrary(BaseLibraryName);

	PROC hBaseProc;

	if (UnHook)
		hBaseProc = (PROC)Custom;
	else
		hBaseProc = GetProcAddress(hBaseLib, BaseFunctionName);

	PFNENUMPROCESSMODULES m_pfnEnumProcessModules;
	HMODULE m_hModPSAPI = ::LoadLibraryA("PSAPI.DLL");

	m_pfnEnumProcessModules = (PFNENUMPROCESSMODULES)::GetProcAddress(m_hModPSAPI, "EnumProcessModules");

	HANDLE hProcess = ::GetCurrentProcess();

	if (m_pfnEnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
	{
		for (i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
		{
			SetHook(hMods[i], BaseLibraryName, hBaseProc, NewFunctionPointer);
		}
	}

	return hBaseProc;
}
#include <time.h>
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		char czTime[MAXCHAR] = { 0 };
		if (sizeof(time_t) > sizeof(long))
		{
			sprintf(czTime, "D:\\init_inject_success_%lld.log", time(0));
		}
		else
		{
			sprintf(czTime, "D:\\init_inject_success_%ld.log", time(0));
		}
		FILE * pFile = fopen(czTime, "wb");
		if (pFile)
		{
			if (sizeof(_TCHAR) > sizeof(BYTE))
			{
				fwrite("\xFF\xFE", sizeof(_TCHAR), sizeof(BYTE), pFile);
			}
			fprintf(pFile, "注入已成功！进入注入模式。\r\n");
			fclose(pFile);
		}
		InitDebugConsole();
		g_hModule = hModule;
		g_OriginalCreateFileW = EnumAndSetHooks("KERNEL32.DLL", "CreateFileW", (PROC)HookCreateFileW, false, 0);
		g_OriginalCopyFileW = EnumAndSetHooks("KERNEL32.DLL", "CopyFileW", (PROC)HookCopyFileW, false, 0);
	}
	break;
	case DLL_THREAD_ATTACH:
	{

	}
	break;
	case DLL_THREAD_DETACH:
	{

	}
	break;
	case DLL_PROCESS_DETACH:
	{
		char czTime[MAXCHAR] = { 0 };
		if (sizeof(time_t) > sizeof(long))
		{
			sprintf(czTime, "D:\\exit_inject_success_%lld.log", time(0));
		}
		else
		{
			sprintf(czTime, "D:\\exit_inject_success_%ld.log", time(0));
		}
		FILE * pFile = fopen(czTime, "wb");
		if (pFile)
		{
			if (sizeof(_TCHAR) > sizeof(BYTE))
			{
				fwrite("\xFF\xFE", sizeof(_TCHAR), sizeof(BYTE), pFile);
			}
			fprintf(pFile, "注入已成功！进入注入模式。\r\n");
			fclose(pFile);
		}
		EnumAndSetHooks("KERNEL32.DLL", "CreateFileW", (PROC)GetProcAddress(LoadLibrary("KERNEL32"), "CreateFileW"), true, (PROC)HookCreateFileW);
		EnumAndSetHooks("KERNEL32.DLL", "CopyFileW", (PROC)GetProcAddress(LoadLibrary("KERNEL32"), "CopyFileW"), true, (PROC)HookCopyFileW);
		ExitDebugConsole();
	}
	break;
		break;
	}
	return TRUE;
}

