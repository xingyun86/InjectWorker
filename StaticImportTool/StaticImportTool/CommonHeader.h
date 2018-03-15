
#include <windows.h>
#include <stdlib.h>
#include <conio.h>

namespace UNK_API{
	typedef enum _PROCESSINFOCLASSEX
	{
		ProcessHandleInformation = 20,
	}PROCESSINFOCLASSEX;
	typedef LONG NTSTATUS;

#define STATUS_SUCCESS                  ((NTSTATUS)0x00000000L)   
#define STATUS_UNSUCCESSFUL             ((NTSTATUS)0xC0000001L)   
#define STATUS_NOT_IMPLEMENTED          ((NTSTATUS)0xC0000002L)   
#define STATUS_INVALID_INFO_CLASS       ((NTSTATUS)0xC0000003L)   
#define STATUS_INFO_LENGTH_MISMATCH     ((NTSTATUS)0xC0000004L)   

	typedef enum _SYSTEM_INFORMATION_CLASS
	{
		SystemBasicInformation,                 //  0 Y N   
		SystemProcessorInformation,             //  1 Y N   
		SystemPerformanceInformation,           //  2 Y N   
		SystemTimeOfDayInformation,             //  3 Y N   
		SystemNotImplemented1,                  //  4 Y N   
		SystemProcessesAndThreadsInformation,   //  5 Y N   
		SystemCallCounts,                       //  6 Y N   
		SystemConfigurationInformation,         //  7 Y N   
		SystemProcessorTimes,                   //  8 Y N   
		SystemGlobalFlag,                       //  9 Y Y   
		SystemNotImplemented2,                  // 10 Y N   
		SystemModuleInformation,                // 11 Y N   
		SystemLockInformation,                  // 12 Y N   
		SystemNotImplemented3,                  // 13 Y N   
		SystemNotImplemented4,                  // 14 Y N   
		SystemNotImplemented5,                  // 15 Y N   
		SystemHandleInformation,                // 16 Y N   
		SystemObjectInformation,                // 17 Y N   
		SystemPagefileInformation,              // 18 Y N   
		SystemInstructionEmulationCounts,       // 19 Y N   
		SystemInvalidInfoClass1,                // 20   
		SystemCacheInformation,                 // 21 Y Y   
		SystemPoolTagInformation,               // 22 Y N   
		SystemProcessorStatistics,              // 23 Y N   
		SystemDpcInformation,                   // 24 Y Y   
		SystemNotImplemented6,                  // 25 Y N   
		SystemLoadImage,                        // 26 N Y   
		SystemUnloadImage,                      // 27 N Y   
		SystemTimeAdjustment,                   // 28 Y Y   
		SystemNotImplemented7,                  // 29 Y N   
		SystemNotImplemented8,                  // 30 Y N   
		SystemNotImplemented9,                  // 31 Y N   
		SystemCrashDumpInformation,             // 32 Y N   
		SystemExceptionInformation,             // 33 Y N   
		SystemCrashDumpStateInformation,        // 34 Y Y/N   
		SystemKernelDebuggerInformation,        // 35 Y N   
		SystemContextSwitchInformation,         // 36 Y N   
		SystemRegistryQuotaInformation,         // 37 Y Y   
		SystemLoadAndCallImage,                 // 38 N Y   
		SystemPrioritySeparation,               // 39 N Y   
		SystemNotImplemented10,                 // 40 Y N   
		SystemNotImplemented11,                 // 41 Y N   
		SystemInvalidInfoClass2,                // 42   
		SystemInvalidInfoClass3,                // 43   
		SystemTimeZoneInformation,              // 44 Y N   
		SystemLookasideInformation,             // 45 Y N   
		SystemSetTimeSlipEvent,                 // 46 N Y   
		SystemCreateSession,                    // 47 N Y   
		SystemDeleteSession,                    // 48 N Y   
		SystemInvalidInfoClass4,                // 49   
		SystemRangeStartInformation,            // 50 Y N   
		SystemVerifierInformation,              // 51 Y Y   
		SystemAddVerifier,                      // 52 N Y   
		SystemSessionProcessesInformation       // 53 Y N   

	} SYSTEM_INFORMATION_CLASS;

	typedef struct _LSA_UNICODE_STRING
	{
		USHORT Length;
		USHORT MaximumLength;
		PWSTR Buffer;

	} LSA_UNICODE_STRING, *PLSA_UNICODE_STRING, UNICODE_STRING, *PUNICODE_STRING;

	typedef struct _CLIENT_ID
	{
		HANDLE UniqueProcess;
		HANDLE UniqueThread;

	} CLIENT_ID, *PCLIENT_ID;

	typedef struct _OBJECT_ATTRIBUTES {
		ULONG           Length;
		HANDLE          RootDirectory;
		PUNICODE_STRING ObjectName;
		ULONG           Attributes;
		PVOID           SecurityDescriptor;
		PVOID           SecurityQualityOfService;
	}  OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;

	typedef enum _THREAD_STATE
	{
		StateInitialized,
		StateReady,
		StateRunning,
		StateStandby,
		StateTerminated,
		StateWait,
		StateTransition,
		StateUnknown

	} THREAD_STATE;

	typedef enum _KWAIT_REASON
	{
		Executive,
		FreePage,
		PageIn,
		PoolAllocation,
		DelayExecution,
		Suspended,
		UserRequest,
		WrExecutive,
		WrFreePage,
		WrPageIn,
		WrPoolAllocation,
		WrDelayExecution,
		WrSuspended,
		WrUserRequest,
		WrEventPair,
		WrQueue,
		WrLpcReceive,
		WrLpcReply,
		WrVirtualMemory,
		WrPageOut,
		WrRendezvous,
		Spare2,
		Spare3,
		Spare4,
		Spare5,
		Spare6,
		WrKernel

	} KWAIT_REASON;

	/*typedef struct _IO_COUNTERS
	{
	LARGE_INTEGER ReadOperationCount;   //I/O读操作数目
	LARGE_INTEGER WriteOperationCount;  //I/O写操作数目
	LARGE_INTEGER OtherOperationCount;  //I/O其他操作数目
	LARGE_INTEGER ReadTransferCount;    //I/O读数据数目
	LARGE_INTEGER WriteTransferCount;   //I/O写数据数目
	LARGE_INTEGER OtherTransferCount;   //I/O其他操作数据数目

	} IO_COUNTERS, *PIO_COUNTERS;
	*/
	typedef struct _VM_COUNTERS
	{
		ULONG PeakVirtualSize;              //虚拟存储峰值大小   
		ULONG VirtualSize;                  //虚拟存储大小   
		ULONG PageFaultCount;               //页故障数目   
		ULONG PeakWorkingSetSize;           //工作集峰值大小   
		ULONG WorkingSetSize;               //工作集大小   
		ULONG QuotaPeakPagedPoolUsage;      //分页池使用配额峰值   
		ULONG QuotaPagedPoolUsage;          //分页池使用配额   
		ULONG QuotaPeakNonPagedPoolUsage;   //非分页池使用配额峰值   
		ULONG QuotaNonPagedPoolUsage;       //非分页池使用配额   
		ULONG PagefileUsage;                //页文件使用情况   
		ULONG PeakPagefileUsage;            //页文件使用峰值   

	} VM_COUNTERS, *PVM_COUNTERS;

	typedef LONG KPRIORITY;

	typedef struct _SYSTEM_THREADS32
	{
		LARGE_INTEGER KernelTime;
		LARGE_INTEGER UserTime;
		LARGE_INTEGER CreateTime;
		ULONG WaitTime;
		PVOID StartAddress;
		CLIENT_ID ClientId;
		KPRIORITY Priority;
		KPRIORITY BasePriority;
		ULONG ContextSwitchCount;
		THREAD_STATE State;
		KWAIT_REASON WaitReason;
	} SYSTEM_THREADS32, *PSYSTEM_THREADS32;

	typedef struct _SYSTEM_THREADS64
	{
		LARGE_INTEGER KernelTime;
		LARGE_INTEGER UserTime;
		LARGE_INTEGER CreateTime;
		ULONG WaitTime;
		PVOID StartAddress;
		CLIENT_ID ClientId;
		KPRIORITY Priority;
		KPRIORITY BasePriority;
		ULONG ContextSwitchCount;
		THREAD_STATE State;
		KWAIT_REASON WaitReason;
		ULONG Reserved;
	} SYSTEM_THREADS64, *PSYSTEM_THREADS64;

	typedef struct _SYSTEM_PROCESSES32
	{
		ULONG NextEntryDelta;
		ULONG ThreadCount;
		ULONG Reserved1[6];
		LARGE_INTEGER CreateTime;
		LARGE_INTEGER UserTime;
		LARGE_INTEGER KernelTime;
		UNICODE_STRING ProcessName;
		KPRIORITY BasePriority;
		ULONG ProcessId;
		ULONG InheritedFromProcessId;
		ULONG HandleCount;
		ULONG Reserved2[2];
		VM_COUNTERS VmCounters;
		IO_COUNTERS IoCounters;
		SYSTEM_THREADS32 Threads[1];
	} SYSTEM_PROCESSES32, *PSYSTEM_PROCESSES32;

	typedef struct _SYSTEM_PROCESSES64
	{
		ULONG NextEntryDelta;
		ULONG ThreadCount;
		ULONG Reserved1[6];
		LARGE_INTEGER CreateTime;
		LARGE_INTEGER UserTime;
		LARGE_INTEGER KernelTime;
		UNICODE_STRING ProcessName;
		KPRIORITY BasePriority;
		HANDLE ProcessId;
		HANDLE InheritedFromProcessId;
		ULONG HandleCount;
		ULONG Reserved2[2];
		SIZE_T PageDirectoryBase;
		VM_COUNTERS VmCounters;
		SIZE_T PrivatePageCount;
		IO_COUNTERS IoCounters;
		SYSTEM_THREADS64 Threads[1];
	} SYSTEM_PROCESSES64, *PSYSTEM_PROCESSES64;

	typedef struct _SYSTEM_BASIC_INFORMATION
	{
		BYTE Reserved1[24];
		PVOID Reserved2[4];
		CCHAR NumberOfProcessors;

	} SYSTEM_BASIC_INFORMATION, *PSYSTEM_BASIC_INFORMATION;

	typedef struct _SYSTEM_MODULE_INFORMATION32 {
		ULONG Reserved[2];
		PVOID Base;
		ULONG Size;
		ULONG Flags;
		USHORT Index;
		USHORT Unknown;
		USHORT LoadCount;
		USHORT ModuleNameOffset;
		CHAR ImageName[256];
	} SYSTEM_MODULE_INFORMATION32, *PSYSTEM_MODULE_INFORMATION32;

	typedef struct _SYSTEM_MODULE_INFORMATION64 {
		ULONG Reserved[3];
		PVOID Base;
		ULONG Size;
		ULONG Flags;
		USHORT Index;
		USHORT Unknown;
		USHORT LoadCount;
		USHORT ModuleNameOffset;
		CHAR ImageName[256];
	} SYSTEM_MODULE_INFORMATION64, *PSYSTEM_MODULE_INFORMATION64;

#if !defined(_WIN64) && !defined(WIN64)
	typedef _SYSTEM_MODULE_INFORMATION32 _SYSTEM_MODULE_INFORMATION;
	typedef SYSTEM_MODULE_INFORMATION32 SYSTEM_MODULE_INFORMATION;
	typedef PSYSTEM_MODULE_INFORMATION32 PSYSTEM_MODULE_INFORMATION;

	typedef _SYSTEM_THREADS32 _SYSTEM_THREADS;
	typedef SYSTEM_THREADS32 SYSTEM_THREADS;
	typedef PSYSTEM_THREADS32 PSYSTEM_THREADS;

	typedef _SYSTEM_PROCESSES32 _SYSTEM_PROCESSES;
	typedef SYSTEM_PROCESSES32 SYSTEM_PROCESSES;
	typedef PSYSTEM_PROCESSES32 PSYSTEM_PROCESSES;
#else
	typedef _SYSTEM_MODULE_INFORMATION64 _SYSTEM_MODULE_INFORMATION;
	typedef SYSTEM_MODULE_INFORMATION64 SYSTEM_MODULE_INFORMATION;
	typedef PSYSTEM_MODULE_INFORMATION64 PSYSTEM_MODULE_INFORMATION;

	typedef _SYSTEM_THREADS64 _SYSTEM_THREADS;
	typedef SYSTEM_THREADS64 SYSTEM_THREADS;
	typedef PSYSTEM_THREADS64 PSYSTEM_THREADS;

	typedef _SYSTEM_PROCESSES64 _SYSTEM_PROCESSES;
	typedef SYSTEM_PROCESSES64 SYSTEM_PROCESSES;
	typedef PSYSTEM_PROCESSES64 PSYSTEM_PROCESSES;
#endif // !defined(_WIN64) && !defined(WIN64)

	typedef enum _OBJECT_INFORMATION_CLASSEX {
		ObjectBasicInformation = 0,
		ObjectNameInformation,
		ObjectTypeInformation,
		ObjectAllInformation,
		ObjectDataInformation,
	} OBJECT_INFORMATION_CLASSEX;

	///////////////////////////////////////////////////////////////////
	//VOID InitializeObjectAttributes(
	//		[out]          POBJECT_ATTRIBUTES   InitializedAttributes,
	//		[in]           PUNICODE_STRING      ObjectName,
	//		[in]           ULONG                Attributes,
	//		[in]           HANDLE               RootDirectory,
	//		[in, optional] PSECURITY_DESCRIPTOR SecurityDescriptor
	//		);
	// 初始化对象属性宏定义
	VOID InitializeObjectAttributes(
		POBJECT_ATTRIBUTES   InitializedAttributes,
		PUNICODE_STRING      ObjectName,
		ULONG                Attributes,
		HANDLE               RootDirectory,
		PSECURITY_DESCRIPTOR SecurityDescriptor
		)
	{
		InitializedAttributes->Length = sizeof(OBJECT_ATTRIBUTES);
		InitializedAttributes->ObjectName = ObjectName;
		InitializedAttributes->Attributes = Attributes;
		InitializedAttributes->RootDirectory = RootDirectory;
		InitializedAttributes->SecurityDescriptor = SecurityDescriptor;
		InitializedAttributes->SecurityQualityOfService = NULL;
	}

	typedef NTSTATUS(NTAPI *LPFN_ZwCreateThreadEx)(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, HANDLE, PVOID, PVOID, BOOL, SIZE_T, SIZE_T, SIZE_T, PVOID);
	typedef DWORD(WINAPI *LPFN_ZwSuspendProcess)(HANDLE);
	typedef DWORD(WINAPI *LPFN_ZwResumeProcess)(HANDLE);
	typedef NTSTATUS(NTAPI *LPFN_ZwQueryInformationProcess)(HANDLE, PROCESSINFOCLASSEX, LPVOID, DWORD, PDWORD);
	typedef NTSTATUS(NTAPI *LPFN_ZwQuerySystemInformation)(DWORD, PVOID, DWORD, DWORD*);
	typedef NTSTATUS(NTAPI *LPFN_ZwQueryObject)(HANDLE, OBJECT_INFORMATION_CLASSEX, PVOID, ULONG, PULONG);
	typedef NTSTATUS(NTAPI *LPFN_RtlAdjustPrivilege)(DWORD, BOOLEAN, BOOLEAN, PBOOLEAN);
	typedef NTSTATUS(NTAPI *LPFN_RtlCreateUserThread)(HANDLE, PSECURITY_DESCRIPTOR, BOOLEAN, ULONG, SIZE_T, SIZE_T, PVOID, PVOID, PHANDLE, PCLIENT_ID);
	typedef NTSTATUS(NTAPI *LPFN_NtOpenProcess)(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PCLIENT_ID);
	typedef NTSTATUS(NTAPI *LPFN_NtWriteVirtualMemory)(HANDLE, PVOID, PVOID, ULONG, PULONG);
	typedef NTSTATUS(NTAPI *LPFN_NtWaitForSingleObject)(HANDLE, BOOLEAN, PLARGE_INTEGER);
	typedef NTSTATUS(NTAPI *LPFN_NtClose)(HANDLE);
	typedef NTSTATUS(NTAPI *LPFN_CsrGetProcessId)(void);
	typedef NTSTATUS(NTAPI *LPFN_RtlInitUnicodeString)(PUNICODE_STRING, PCWSTR);
	typedef NTSTATUS(NTAPI *LPFN_LdrLoadDll)(PWCHAR, PULONG, PUNICODE_STRING, PHANDLE);
	typedef DWORD(WINAPI *LPFN_GetTempPathW)(_In_ DWORD, LPWSTR);
	typedef UINT(WINAPI *LPFN_GetSystemDirectoryW)(LPWSTR, UINT);
	typedef BOOL(WINAPI *LPFN_GetVolumeInformationW)(LPCWSTR, LPWSTR, DWORD, LPDWORD, LPDWORD, LPDWORD, LPWSTR, DWORD);
	typedef DWORD(WINAPI* LPFN_GetModuleFileNameW)(HMODULE, LPWSTR, DWORD);
	typedef HRESULT(STDAPICALLTYPE* LPFN_PathRemoveFileSpecW)(LPWSTR);


	LPFN_ZwCreateThreadEx FN_ZwCreateThreadEx = nullptr;
	LPFN_ZwSuspendProcess FN_ZwSuspendProcess = nullptr;//挂起进程的API 
	LPFN_ZwResumeProcess FN_ZwResumeProcess = nullptr;//恢复进程的API
	LPFN_ZwQuerySystemInformation FN_ZwQuerySystemInformation = nullptr;
	LPFN_RtlAdjustPrivilege FN_RtlAdjustPrivilege = nullptr;
	LPFN_RtlCreateUserThread FN_RtlCreateUserThread = nullptr;
	LPFN_NtOpenProcess FN_NtOpenProcess = nullptr;
	LPFN_NtWriteVirtualMemory FN_NtWriteVirtualMemory = nullptr;
	LPFN_NtWaitForSingleObject FN_NtWaitForSingleObject = nullptr;
	LPFN_NtClose FN_NtClose = nullptr;
	LPFN_CsrGetProcessId FN_CsrGetProcessId = nullptr;
	LPFN_RtlInitUnicodeString FN_RtlInitUnicodeString = nullptr;
	LPFN_LdrLoadDll FN_LdrLoadDll = nullptr;
	LPFN_GetTempPathW FN_GetTempPathW = nullptr;
	LPFN_GetSystemDirectoryW FN_GetSystemDirectoryW = nullptr;
	LPFN_GetVolumeInformationW FN_GetVolumeInformationW = nullptr;
	LPFN_GetModuleFileNameW FN_GetModuleFileNameW = nullptr;
	LPFN_PathRemoveFileSpecW FN_PathRemoveFileSpecW = nullptr;
	void InitApis()
	{
		const _TCHAR * NTDLL_NAME = _T("NTDLL.DLL");
		const _TCHAR * KERNEL32_NAME = _T("KERNEL32.DLL");
		const _TCHAR * SHLWAPI_NAME = _T("SHLWAPI.DLL");
		HMODULE hModuleNTDLL = GetModuleHandle(NTDLL_NAME);
		HMODULE hModuleKERNEL32 = GetModuleHandle(KERNEL32_NAME);
		HMODULE hModuleSHLWAPI = GetModuleHandle(SHLWAPI_NAME);
		if (!hModuleNTDLL)
		{
			hModuleNTDLL = LoadLibrary(NTDLL_NAME);
		}
		if (!hModuleKERNEL32)
		{
			hModuleKERNEL32 = LoadLibrary(KERNEL32_NAME);
		}
		if (!hModuleSHLWAPI)
		{
			hModuleSHLWAPI = LoadLibrary(SHLWAPI_NAME);
		}
		FN_ZwCreateThreadEx = (LPFN_ZwCreateThreadEx)GetProcAddress(hModuleNTDLL, "ZwCreateThreadEx");
		FN_ZwSuspendProcess = (LPFN_ZwSuspendProcess)GetProcAddress(hModuleNTDLL, "ZwSuspendProcess");
		FN_ZwResumeProcess = (LPFN_ZwResumeProcess)GetProcAddress(hModuleNTDLL, "ZwResumeProcess");
		FN_ZwQuerySystemInformation = (LPFN_ZwQuerySystemInformation)GetProcAddress(hModuleNTDLL, "ZwQuerySystemInformation");
		FN_RtlAdjustPrivilege = (LPFN_RtlAdjustPrivilege)GetProcAddress(hModuleNTDLL, "RtlAdjustPrivilege");
		FN_RtlCreateUserThread = (LPFN_RtlCreateUserThread)GetProcAddress(hModuleNTDLL, "RtlCreateUserThread");
		FN_NtOpenProcess = (LPFN_NtOpenProcess)GetProcAddress(hModuleNTDLL, "NtOpenProcess");
		FN_NtWriteVirtualMemory = (LPFN_NtWriteVirtualMemory)GetProcAddress(hModuleNTDLL, "NtWriteVirtualMemory");
		FN_NtWaitForSingleObject = (LPFN_NtWaitForSingleObject)GetProcAddress(hModuleNTDLL, "NtWaitForSingleObject");
		FN_NtClose = (LPFN_NtClose)GetProcAddress(hModuleNTDLL, "NtClose");
		FN_CsrGetProcessId = (LPFN_CsrGetProcessId)GetProcAddress(hModuleNTDLL, "CsrGetProcessId");
		FN_RtlInitUnicodeString = (LPFN_RtlInitUnicodeString)GetProcAddress(hModuleNTDLL, "RtlInitUnicodeString");
		FN_LdrLoadDll = (LPFN_LdrLoadDll)GetProcAddress(hModuleNTDLL, "LdrLoadDll");
		FN_GetTempPathW = (LPFN_GetTempPathW)GetProcAddress(hModuleKERNEL32, "GetTempPathW");
		FN_GetSystemDirectoryW = (LPFN_GetSystemDirectoryW)GetProcAddress(hModuleKERNEL32, "GetSystemDirectoryW");
		FN_GetVolumeInformationW = (LPFN_GetVolumeInformationW)GetProcAddress(hModuleKERNEL32, "GetVolumeInformationW");
		FN_GetModuleFileNameW = (LPFN_GetModuleFileNameW)GetProcAddress(hModuleKERNEL32, "GetModuleFileNameW");
		FN_PathRemoveFileSpecW = (LPFN_PathRemoveFileSpecW)GetProcAddress(hModuleSHLWAPI, "PathRemoveFileSpecW");
		
		printf("FN_ZwCreateThreadEx=0x%08X\r\n", FN_ZwCreateThreadEx);
		printf("FN_ZwSuspendProcess=0x%08X\r\n", FN_ZwSuspendProcess);
		printf("FN_ZwResumeProcess=0x%08X\r\n", FN_ZwResumeProcess);
		printf("FN_ZwQuerySystemInformation=0x%08X\r\n", FN_ZwQuerySystemInformation);
		printf("FN_RtlAdjustPrivilege=0x%08X\r\n", FN_RtlAdjustPrivilege);
		printf("FN_RtlCreateUserThread=0x%08X\r\n", FN_RtlCreateUserThread);
		printf("FN_NtOpenProcess=0x%08X\r\n", FN_NtOpenProcess);
		printf("FN_NtWriteVirtualMemory=0x%08X\r\n", FN_NtWriteVirtualMemory);
		printf("FN_NtWaitForSingleObject=0x%08X\r\n", FN_NtWaitForSingleObject);
		printf("FN_NtClose=0x%08X\r\n", FN_NtClose);
		printf("FN_CsrGetProcessId=0x%08X\r\n", FN_CsrGetProcessId);
		printf("FN_RtlInitUnicodeString=0x%08X\r\n", FN_RtlInitUnicodeString);
		printf("FN_LdrLoadDll=0x%08X\r\n", FN_LdrLoadDll);
		printf("FN_GetTempPathW=0x%08X\r\n", FN_GetTempPathW);
		printf("FN_GetSystemDirectoryW=0x%08X\r\n", FN_GetSystemDirectoryW);
		printf("FN_GetVolumeInformationW=0x%08X\r\n", FN_GetVolumeInformationW);
		printf("FN_GetModuleFileNameW=0x%08X\r\n", FN_GetModuleFileNameW);
		printf("FN_PathRemoveFileSpecW=0x%08X\r\n", FN_PathRemoveFileSpecW);
	}
	//操作系统版本判断
	BOOL IsVistaOrLater()
	{
		OSVERSIONINFO osvi;
		ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx(&osvi);
		if (osvi.dwMajorVersion >= 6)
			return TRUE;
		return FALSE;
	}
	//获取进程挂起/运行状态
	BOOL GetProcessState(DWORD dwProcessId)
	{
		NTSTATUS status = STATUS_UNSUCCESSFUL;

		ULONG cbSize = 0;
		BYTE *pvData = NULL;

		//ShowTips(_T("---------------------所有进程信息----------------------------------------\n"));
		PSYSTEM_PROCESSES psp = NULL;
		status = (NTSTATUS)FN_ZwQuerySystemInformation(SystemProcessesAndThreadsInformation, NULL, 0, &cbSize);
		if (status == STATUS_INFO_LENGTH_MISMATCH) {
			pvData = (BYTE *)malloc(cbSize * sizeof(BYTE));
			status = (NTSTATUS)FN_ZwQuerySystemInformation(SystemProcessesAndThreadsInformation, (PVOID)pvData, cbSize, NULL);
			if (status == STATUS_SUCCESS)
			{
				psp = (PSYSTEM_PROCESSES)pvData;
				do {
					if ((DWORD)psp->ProcessId == (DWORD)dwProcessId)
					{
						if (psp->Threads[0].State == StateWait && psp->Threads[0].WaitReason == Suspended)
						{
							return TRUE;
						}
						else
						{
							return FALSE;
						}
					}
					psp = (PSYSTEM_PROCESSES)(SIZE_T)((SIZE_T)psp + psp->NextEntryDelta);
				} while (psp->NextEntryDelta != 0);

				free(pvData);
				pvData = NULL;
			}
			else if (status == STATUS_UNSUCCESSFUL) {
				//ShowMsgs(_T("提示"), _T("\n STATUS_UNSUCCESSFUL"));
			}
			else if (status == STATUS_NOT_IMPLEMENTED) {
				//ShowMsgs(_T("提示"), _T("\n STATUS_NOT_IMPLEMENTED"));
			}
			else if (status == STATUS_INVALID_INFO_CLASS) {
				//ShowMsgs(_T("提示"), _T("\n STATUS_INVALID_INFO_CLASS"));
			}
			else if (status == STATUS_INFO_LENGTH_MISMATCH) {
				//ShowMsgs(_T("提示"), _T("\n STATUS_INFO_LENGTH_MISMATCH"));
			}
		}

		return FALSE;
	}

	int GetNameFromPath(_TCHAR(&Name)[MAX_PATH], const _TCHAR * pPath)
	{
		int result = (-1);
		_TCHAR * pT = NULL;

		if (pPath && *pPath)
		{
			pT = _tcsrchr((_TCHAR *)pPath, _T('\\'));
			if (!pT)
			{
				pT = _tcsrchr((_TCHAR *)pPath, _T('/'));
			}
			if (pT && *pT)
			{
				_tcscpy(Name, pT + 1);
				result = 0;
			}
		}

		return result;
	}
	//提升进程访问权限
	bool EnableDebugPriv()
	{
		HANDLE hToken;
		LUID sedebugnameValue;
		TOKEN_PRIVILEGES tkp;

		if (!OpenProcessToken(GetCurrentProcess(),
			TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
			return false;
		}

		if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &sedebugnameValue)) {
			CloseHandle(hToken);
			return false;
		}

		tkp.PrivilegeCount = 1;
		tkp.Privileges[0].Luid = sedebugnameValue;
		tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

		if (!AdjustTokenPrivileges(hToken, FALSE, &tkp, sizeof(tkp), NULL, NULL)) {
			CloseHandle(hToken);
			return false;
		}

		return true;
	}
	//进程快照（枚举各进程）
	BOOL GetPidByProcessName(LPCTSTR lpszProcessName, DWORD & dwProcessId)
	{
		HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (INVALID_HANDLE_VALUE == hSnapshot)
		{
			return FALSE;
		}

		PROCESSENTRY32 pe;
		pe.dwSize = sizeof(PROCESSENTRY32);
		if (!Process32First(hSnapshot, &pe))
		{
			::CloseHandle(hSnapshot);
			return FALSE;
		}

		while (Process32Next(hSnapshot, &pe))
		{
			if (!_tcsicmp(lpszProcessName, pe.szExeFile))
			{
				::CloseHandle(hSnapshot);
				dwProcessId = pe.th32ProcessID;
				return TRUE;
			}
		}

		::CloseHandle(hSnapshot);
		return FALSE;
	}

	////////////////////////////////////////////////////////////////////////////////////////////
#define VM_CODEINIT_PAGESIZE	0x1000 //4K=4096
//#pragma push(4)
	typedef struct _THREAD_DATA{
		LPFN_RtlInitUnicodeString fnRtlInitUnicodeString;
		LPFN_LdrLoadDll fnLdrLoadDll;
		LPFN_GetTempPathW fnGetTempPathW;
		LPFN_GetSystemDirectoryW fnGetSystemDirectoryW;
		LPFN_GetVolumeInformationW fnGetVolumeInformationW;
		WCHAR wDllPath[MAX_PATH];
		WCHAR wDllName[MAX_PATH];
	}THREAD_DATA, *PTHREAD_DATA;
//#pragma pop(4)
	//////////////////////////////////////////////////////////////////////////
	
	static HANDLE WINAPI ThreadRoutine(PTHREAD_DATA pThreadData)
	{
		if (pThreadData->fnRtlInitUnicodeString != nullptr && pThreadData->fnLdrLoadDll != nullptr)
		{
			HANDLE hModule = nullptr;
			UNICODE_STRING UnicodeString = { 0 };
	
			pThreadData->fnRtlInitUnicodeString(&UnicodeString, pThreadData->wDllPath);
			
			return (HANDLE)pThreadData->fnLdrLoadDll(nullptr, nullptr, &UnicodeString, &hModule);
		}
		else
		{
			return (HANDLE)(-3);
		}
	}
	static DWORD WINAPI ThreadRoutineSize()
	{
		return 0;
	}
	static bool SetProcessPrivilege(DWORD SE_DEBUG_PRIVILEGE = 0x14){
		BOOLEAN bl;
		FN_RtlAdjustPrivilege(SE_DEBUG_PRIVILEGE, TRUE, FALSE, &bl);
		return bl;
	}
	static std::wstring GetAbsolutePath(const std::wstring& name){
		WCHAR wFileName[MAX_PATH] = { 0 };
		FN_GetModuleFileNameW(NULL, wFileName, MAX_PATH);
		FN_PathRemoveFileSpecW(wFileName);
		return std::wstring(wFileName).append(name);
	}
	static bool ProcessInternalExecute(PTHREAD_DATA pThreadData, DWORD dwProcessId)
	{
		HANDLE hProcess = nullptr;
		CLIENT_ID cid = { (HANDLE)dwProcessId, NULL };
		OBJECT_ATTRIBUTES oa = { 0 };

		printf("ProcessInternalExecute Enter!\r\n");
		InitializeObjectAttributes(&oa, NULL, 0, NULL, NULL);

		if (FN_NtOpenProcess(&hProcess, PROCESS_ALL_ACCESS, &oa, &cid) != STATUS_SUCCESS)
		{
			printf("FN_NtOpenProcess failed!\r\n");
			return false;
		}
		printf("FN_NtOpenProcess success!\r\n");
		SIZE_T stDataSize = sizeof(THREAD_DATA);// VM_CODEINIT_PAGESIZE;
		PVOID pDataAddr = VirtualAllocEx(hProcess, NULL, stDataSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		if (!pDataAddr)
		{
			printf("VirtualAllocEx failed!\r\n");
			FN_NtClose(hProcess);
			return false;
		}
		printf("VirtualAllocEx success!pDataAddr=0x%X,stDataSize=%llu(0x%X)\r\n", pDataAddr, stDataSize, stDataSize);
		FN_NtWriteVirtualMemory(hProcess, pDataAddr, (PVOID)pThreadData, sizeof(THREAD_DATA), NULL);
		SIZE_T stCodeSize = (ULONG)((LPBYTE)ThreadRoutineSize - (LPBYTE)ThreadRoutine);
		PVOID pCodeAddr = VirtualAllocEx(hProcess, NULL, stCodeSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
		if (!pCodeAddr)
		{
			printf("VirtualAllocEx failed!\r\n");
			FN_NtClose(hProcess);
			return false;
		}
		printf("VirtualAllocEx success!pCodeAddr=0x%X,stCodeSize=%llu(0x%X)\r\n", pCodeAddr, stCodeSize, stCodeSize);
		
		FN_NtWriteVirtualMemory(hProcess, pCodeAddr, (PVOID)ThreadRoutine, stCodeSize, NULL);
		HANDLE hThread = NULL;
		CLIENT_ID cid_tmp = { 0 };
		printf("准备完毕!\r\n按任意键继续注入\r\n");
		getchar();
		
		//HANDLE hThread = 0;
		//NTSTATUS ntStat = FN_ZwCreateThreadEx(&hThread, THREAD_ALL_ACCESS_VISTA, 0, NtCurrentProcess, (LPTHREAD_START_ROUTINE)ContinueExecution, 0, THREAD_CREATE_FLAGS_HIDE_FROM_DEBUGGER, 0, 0, 0, 0);
		/*FN_ZwCreateThreadEx(&hThread, 0x1FFFFF, 0, hProcess, pCodeAddr, pDataAddr, 0, 0, 0, 0, 0);
		if (!hThread)
		{
			printf("注入失败!\r\n按任意键继续注入\r\n");
			FN_NtClose(hProcess);
			return false;
		}*/
		if (FN_RtlCreateUserThread(hProcess, NULL, FALSE, 0, 0, 0, pCodeAddr, pDataAddr, &hThread, &cid_tmp) != STATUS_SUCCESS)
		{
			printf("FN_RtlCreateUserThread failed!\r\n");
			FN_NtClose(hProcess);
			return false;
		}
		printf("注入成功!\r\n按任意键继续注入\r\n");
		getchar();
		printf("FN_RtlCreateUserThread success!\r\n");
		FN_NtWaitForSingleObject(hThread, FALSE, NULL);
		printf("等待对象成功!\r\n按任意键继续注入\r\n");
		getchar();
		printf("FN_NtWaitForSingleObject success!\r\n");
		DWORD exit_code = -1;
		GetExitCodeThread(hThread, &exit_code);
		FN_NtClose(hThread);
		VirtualFreeEx(hProcess, pDataAddr, 0, MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(hProcess, pCodeAddr, 0, MEM_DECOMMIT | MEM_RELEASE);
		FN_NtClose(hProcess);
		return (exit_code == 0);
	}

	void SetShellcodeLdrModulePath(PTHREAD_DATA parameter, const std::wstring& wstrDllName){
		wcscpy_s(parameter->wDllPath, wstrDllName.c_str());
	}
}