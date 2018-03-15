#include <windows.h>
#include <TlHelp32.h>
#include <stdio.h>
#include <string.h>

#define CHECK_NULL_RET(bCondition) if (!bCondition) goto Exit0

BOOL EnableDebugPrivilege(void)
{
	HANDLE hToken;
	TOKEN_PRIVILEGES tkp;
	BOOL bRet = FALSE;

	bRet = OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);
	CHECK_NULL_RET(bRet);

	bRet = LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tkp.Privileges[0].Luid);
	CHECK_NULL_RET(bRet);
	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	bRet = AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);
	CHECK_NULL_RET(bRet);
	bRet = TRUE;

Exit0:
	CloseHandle(hToken);
	return bRet;
}

BOOL ApcInject(DWORD dwPid, CHAR *pszDllPath)
{
	FILE * pFile = NULL;
	HANDLE hProcess = NULL;
	BOOL bRet = FALSE;
	HANDLE hSnapshot = NULL;
	HANDLE hThread = NULL;
	LPVOID lpDllName = NULL;
	SIZE_T stNumberOfBytesWritten = 0;
	DWORD dwResult = 0;
	THREADENTRY32 te32 = { 0 };

	bRet = EnableDebugPrivilege();
	CHECK_NULL_RET(bRet);

	hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPid);
	CHECK_NULL_RET(bRet);

	lpDllName = VirtualAllocEx(hProcess, NULL, strlen(pszDllPath) + 1,
		MEM_COMMIT, PAGE_READWRITE);
	CHECK_NULL_RET(lpDllName);

	bRet = WriteProcessMemory(hProcess, lpDllName, (LPVOID)pszDllPath,
		strlen(pszDllPath) + 1, &stNumberOfBytesWritten);

	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, dwPid);
	CHECK_NULL_RET((hSnapshot != INVALID_HANDLE_VALUE));

	te32.dwSize = sizeof(THREADENTRY32);
	bRet = Thread32First(hSnapshot, &te32);
	pFile = fopen("D:\\inject_action.log", "wb");
	while (bRet)
	{
		if (te32.th32OwnerProcessID == dwPid)
		{
			hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, te32.th32ThreadID);
			if (hThread)
			{
				dwResult = QueueUserAPC((PAPCFUNC)LoadLibraryA, hThread, (ULONG_PTR)lpDllName);
				printf("%lu-0x%X--\n", dwResult, GetLastError());
				fprintf(pFile, "%lu-0x%X--\r\n", dwResult, GetLastError());
				CloseHandle(hThread);
			}
		}
		te32.dwSize = sizeof(THREADENTRY32);
		bRet = Thread32Next(hSnapshot, &te32);
	}

Exit0:
	if (pFile)
	{
		fclose(pFile);
	}
	// VirtualFreeEx
	CloseHandle(hSnapshot);
	CloseHandle(hProcess);

	// Do NOT check this value
	return bRet;
}

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		printf("Usage: %s PID DllPath\n", argv[0]);
		return 1;
	}

	ApcInject(atoi(argv[1]), argv[2]);
	return 0;
}