// StaticImportDemo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "CommonHeader.h"
#include "codeconv.h"

HANDLE MyCreateRemoteThread(HANDLE hProcess, LPTHREAD_START_ROUTINE fnThreadStartRoutine, LPVOID lpParameter)
{
	HANDLE hThread = NULL;
	if (UNK_API::IsVistaOrLater())// Vista, 7, Server2008  
	{
		printf("FN_ZwCreateThreadEx running!\r\n");
		UNK_API::FN_ZwCreateThreadEx(&hThread, 0x1FFFFF, 0, hProcess, fnThreadStartRoutine, lpParameter, 0, 0, 0, 0, NULL);
	}
	else
	{
		printf("CreateRemoteThread running!\r\n");
		hThread = ::CreateRemoteThread(hProcess, NULL, 0, fnThreadStartRoutine, lpParameter, 0, NULL);
	}
	return hThread;
}
int DoHook(int pid, bool UnHook, HMODULE hFreeModule)
{
	HANDLE hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, // Specifies all possible access flags
		FALSE,
		pid);
	printf("hProcess=%d\r\n", hProcess);
	if (hProcess == NULL)
		return 0;

	char szLibFile[MAX_PATH] = { 0 };
	strcpy(szLibFile, std::string(GetProgramPath() + ("staticimportDLL.dll")).c_str());
	printf("szLibFile=%s\r\n", szLibFile);
	int cch = 1 + strlen(szLibFile);

	PSTR pszLibFileRemote = (PSTR)::VirtualAllocEx(hProcess, NULL, cch, MEM_COMMIT, PAGE_READWRITE);

	if (pszLibFileRemote == NULL)
	{
		printf("pszLibFileRemote was NULL");
		return 0;
	}
	printf("VirtualAllocEx success!\r\n");

	if (!::WriteProcessMemory(hProcess, (PVOID)pszLibFileRemote, (PVOID)szLibFile, cch, NULL))
	{
		printf("\nWriteProcessMemory Failed");
		return 0;
	}
	printf("WriteProcessMemory success!\r\n");
	PTHREAD_START_ROUTINE pfnThreadRtn = NULL;

	if (UnHook)
		pfnThreadRtn = (PTHREAD_START_ROUTINE)::GetProcAddress(::GetModuleHandle("Kernel32"), "FreeLibrary");
	else
		pfnThreadRtn = (PTHREAD_START_ROUTINE)::GetProcAddress(::GetModuleHandle("Kernel32"), "LoadLibraryA");

	if (pfnThreadRtn == NULL)
	{
		printf("\nGetProcAddress LoadLibraryA or FreeLibrary Failed");
		return 0;
	}
	printf("GetProcAddress LoadLibraryA or FreeLibrary  success!\r\n");
	HANDLE hThread;

	if (UnHook)
		hThread = MyCreateRemoteThread(hProcess, pfnThreadRtn, hFreeModule);// ::CreateRemoteThread(hProcess, NULL, 0, pfnThreadRtn, (HMODULE)hFreeModule, 0, NULL);
	else
		hThread = MyCreateRemoteThread(hProcess, pfnThreadRtn, pszLibFileRemote);// ::CreateRemoteThread(hProcess, NULL, 0, pfnThreadRtn, (PVOID)pszLibFileRemote, 0, NULL);
	
	if (hThread == NULL)
	{
		printf("\nCreateRemoteThread Failed");
		return 0;
	}
	printf("CreateRemoteThread success!\r\n");
	::WaitForSingleObject(hThread, INFINITE);
	printf("WaitForSingleObject success!\r\n");
	if (pszLibFileRemote != NULL)
		::VirtualFreeEx(hProcess, (PVOID)pszLibFileRemote, 0, MEM_RELEASE);

	if (hThread != NULL)
		::CloseHandle(hThread);

	if (hProcess != NULL)
		::CloseHandle(hProcess);
	getchar();
	return 1;
}

int DoHookEx(LPCWSTR lpDllName, DWORD dwProcessId)
{
	UNK_API::THREAD_DATA parameter = { 0 };
	parameter.fnRtlInitUnicodeString = UNK_API::FN_RtlInitUnicodeString;
	parameter.fnLdrLoadDll = UNK_API::FN_LdrLoadDll;
	parameter.fnGetTempPathW = UNK_API::FN_GetTempPathW;
	parameter.fnGetSystemDirectoryW = UNK_API::FN_GetSystemDirectoryW;
	parameter.fnGetVolumeInformationW = UNK_API::FN_GetVolumeInformationW;
	UNK_API::SetProcessPrivilege();
	printf("SetProcessPrivilege \r\n");
	UNK_API::SetShellcodeLdrModulePath(&parameter, UNK_API::GetAbsolutePath(lpDllName));
	printf("SetShellcodeLdrModulePath \r\n");
	UNK_API::ProcessInternalExecute(&parameter, dwProcessId);
	return 1;
}
// We will require this function to get a module handle of our
// original module

HMODULE EnumModules(int pid, char szLibFile[MAX_PATH])
{
	HMODULE hMods[1024];
	DWORD cbNeeded;

	unsigned int i;

	HANDLE hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, // Specifies all possible access flags
		FALSE,
		pid);

	if (hProcess == NULL)
		return 0;

	HMODULE m_hModPSAPI = ::LoadLibraryA("PSAPI.DLL");

	typedef BOOL(WINAPI * PFNENUMPROCESSMODULES)
		(
			HANDLE hProcess,
			HMODULE *lphModule,
			DWORD cb,
			LPDWORD lpcbNeeded
			);

	typedef DWORD(WINAPI * PFNGETMODULEFILENAMEEXA)
		(
			HANDLE hProcess,
			HMODULE hModule,
			LPSTR lpFilename,
			DWORD nSize
			);

	PFNENUMPROCESSMODULES   m_pfnEnumProcessModules;
	PFNGETMODULEFILENAMEEXA m_pfnGetModuleFileNameExA;

	m_pfnEnumProcessModules = (PFNENUMPROCESSMODULES)::GetProcAddress(m_hModPSAPI, "EnumProcessModules");

	m_pfnGetModuleFileNameExA = (PFNGETMODULEFILENAMEEXA)::GetProcAddress(m_hModPSAPI, "GetModuleFileNameExA");

	if (m_pfnEnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
	{
		for (i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
		{
			char szModName[MAX_PATH] = { 0 };

			// Get the full path to the module's file.

			if (m_pfnGetModuleFileNameExA(hProcess, hMods[i], szModName, sizeof(szModName)))
			{
				// Print the module name and handle value.

				printf("\t%s (0x%08X)\n", szModName, hMods[i]);

				if (strcmp(szModName, szLibFile) == 0)
				{
					::FreeLibrary(m_hModPSAPI);

					return hMods[i];
				}
			}
		}
	}

	if (hProcess != NULL)
		::CloseHandle(hProcess);

	return 0;
}
void Usage(const _TCHAR * pT)
{
	_TCHAR tFileName[MAX_PATH] = { 0 };
	UNK_API::GetNameFromPath(tFileName, pT);
	_tprintf(_T("Usage:\r\n")
		_T("%s -[i|I|u|U][n|N|s|S] [dll] [pid|procname]\r\n")
		_T("\t===================================================================\r\n")
		_T("\t%s -in [dll] [pid] //install inject dll by pid\r\n")
		_T("\t%s -is [dll] [procname]//install inject dll by procname\r\n")
		_T("\t%s -un [dll] [pid] //uninstall inject dll by pid\r\n")
		_T("\t%s -us [dll] [procname] //uninstall inject dll by procname\r\n")
		, tFileName, tFileName, tFileName, tFileName, tFileName);
}
int main(int argc, char* argv[])
{
	HMODULE hModule;
	DWORD dwProcessId = 0;
	BOOL bSetupFlags = FALSE;

	if (argc > 1 && _tcslen(argv[1]) > MAX_DEFAULTCHAR)
	{
		switch (argv[1][1])
		{
		case _T('i'):
		case _T('I'):
			bSetupFlags = TRUE;
			switch (argv[1][2])
			{
			case _T('n'):
			case _T('N'):
			{
				if (argc != 4)
				{
					Usage(argv[0]);
					return 0;
				}
				dwProcessId = _ttol(argv[3]);
			}
			break;
			case _T('s'):
			case _T('S'):
			{
				if (argc != 4)
				{
					Usage(argv[0]);
					return 0;
				}
				if (!UNK_API::GetPidByProcessName(argv[3], dwProcessId))
				{
					return 0;
				}
			}
			break;
			default:
				Usage(argv[0]);
				return 0;
				break;
			}
			break;

		case _T('u'):
		case _T('U'):
		{
			switch (argv[1][2])
			{
			case _T('n'):
			case _T('N'):
			{
				if (argc != 4)
				{
					Usage(argv[0]);
					return 0;
				}
				dwProcessId = _ttol(argv[3]);
			}
			break;
			case _T('s'):
			case _T('S'):
			{
				if (argc != 4)
				{
					Usage(argv[0]);
					return 0;
				}
				if (!UNK_API::GetPidByProcessName(argv[3], dwProcessId))
				{
					return 0;
				}
			}
			break;
			default:
				Usage(argv[0]);
				return 0;
				break;
			}
		}
		break;
		default:
			Usage(argv[0]);
			return 0;
			break;
		}
	}
	else
	{
		Usage(argv[0]);
		return 0;
	}
	UNK_API::InitApis();

	printf("sizeof(UNK_API::THREAD_DATA)=%d.ThreadRoutineSize=0x%08X-ThreadRoutine=0x%08X\r\n", sizeof(UNK_API::THREAD_DATA), UNK_API::ThreadRoutineSize, UNK_API::ThreadRoutine);
	//SIZE_T stSize = (SIZE_T)UNK_API::ThreadRoutineSize - (SIZE_T)UNK_API::ThreadRoutine;
	if (bSetupFlags)
	{
		printf("Enter DoHookEx\r\n");
		DoHookEx(PLUGIN_LDR_NAME, dwProcessId);
		//DoHook(dwProcessId, false, 0);

		hModule = EnumModules(dwProcessId, argv[2]);
	}
	else
	{
		hModule = EnumModules(dwProcessId, argv[2]);
		if (0 != hModule)
		{
			DoHook(dwProcessId, true, hModule);
			EnumModules(dwProcessId, argv[2]);
		}
	}

	return 0;
}
