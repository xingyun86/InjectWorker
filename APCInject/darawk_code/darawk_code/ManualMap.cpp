//				ManualMap - by Darawk
//
//	v1.1
//	- Fixed a bug that would cause it not to load modules
//	  that were in folders in the PATH environment variable
//	- Fixed a bug that made it so that when ManualMap loaded
//	  a dependency of the main module it was loading, it
//	  wouldn't keep track of them, so GetRemoteModuleHandle()
//	  wouldn't be able to find it them.  This would have
//	  made it impossible for the main module to import
//	  functions from some of it's dependencies.
//	- Also new is that I wrote my own implementation of
//	  GetProcAddress.  Available here:
//	  http://www.darawk.com/Code/GetProcAddress2.cpp
//	  This allows you to dynamically locate functions
//	  exported by ManualMap'd modules.  
//
//	The purpose of ManualMap is to "manually map" a dll
//	module into a remote process's address space.  This
//	means that instead of just manipulating the remote
//	process into calling the LoadLibrary function, we
//	have our own emulation of what LoadLibrary does
//	without all those annoying detectability issues ^^.
//	The advantage of this method over using something 
//	like my CloakDll function, is that this method never 
//	has to call a function like LoadLibrary inside the 
//	remote process.  Since LoadLibrary can be hooked, 
//	the dll	could still be caught at the injection stage. 
//	Or possibly also through the weakness I discussed in 
//	the comment header of that file, which is not present
//	when using this technique.
#include <windows.h>
#include <tlhelp32.h>
#include <shlwapi.h>
#include <vector>

using namespace std;

#pragma comment(lib, "shlwapi.lib")

#define MAX_ENV_LEN 32767

#define IMAGE_DIRECTORY_ENTRY_IMPORT 1
#define IMAGE_DIRECTORY_ENTRY_BASERELOC 5

//	Pietrek's macro
//
//	MakePtr is a macro that allows you to easily add to values (including
//	pointers) together without dealing with C's pointer arithmetic.  It
//	essentially treats the last two parameters as DWORDs.  The first
//	parameter is used to typecast the result to the appropriate pointer type.
#define MakePtr( cast, ptr, addValue ) (cast)( (DWORD_PTR)(ptr) + (DWORD_PTR)(addValue))

//	This one is mine, but obviously..."adapted" from matt's original idea =p
#define MakeDelta(cast, x, y) (cast) ( (DWORD_PTR)(x) - (DWORD_PTR)(y))

HMODULE MapRemoteModule(unsigned long, char *);
HANDLE OpenModule(char *module);

unsigned long GetProcessIdByName(char *);
HMODULE GetRemoteModuleHandle(unsigned long, char *);
FARPROC GetRemoteProcAddress(unsigned long, char *, char *);

bool FixImports(unsigned long, void *, IMAGE_NT_HEADERS *, IMAGE_IMPORT_DESCRIPTOR *);
bool FixRelocs(void *, void *, IMAGE_NT_HEADERS *, IMAGE_BASE_RELOCATION *, unsigned int);
bool MapSections(HANDLE, void *, void *, IMAGE_NT_HEADERS *);

PIMAGE_SECTION_HEADER GetEnclosingSectionHeader(DWORD, PIMAGE_NT_HEADERS);
LPVOID GetPtrFromRVA(DWORD, PIMAGE_NT_HEADERS, PBYTE);

typedef struct MODULE
{
	char name[1024];
	HMODULE handle;
} MODULE, *pMODULE;

//	Vector of the module dependencies that we've loaded
vector<MODULE> mappedMods;

//	Stub that calls the Dll from within the remote process.
//	This is necessary because a DllMain function takes 3
//	arguments, and CreateRemoteThread can pass only 1.
__declspec(naked) void DllCall_stub(HMODULE hMod)
{
	_asm
	{
		push 0
		push 1
		push [esp+0Ch]		//	Pointer to the hMod argument
        mov eax, 0xDEADBEEF	//	Patch this in with the real value at run-time

		call eax			//	MSVC++ doesn't like direct absolute calls, so we have to be
							//	clever about it.

		ret					//	Don't have to clean up the stack because the calling function
							//	is just going to call ExitThread() immediately after this
							//	function returns.
	}
}

//	Marker for the end of the DllCall_stub function
__declspec(naked) void DC_stubend(void) { }	

int main(int argc, char **argv)
{
	//	Just my test values...Cmdline.dll is a plugin that comes with
	//	Olly Debug 1.10
	MapRemoteModule(GetProcessIdByName("notepad.exe"), "Cmdline.dll");
	return 0;
}

HMODULE MapRemoteModule(unsigned long pId, char *module)
{
	IMAGE_DOS_HEADER *dosHd;
	IMAGE_NT_HEADERS *ntHd;

	HANDLE hFile = OpenModule(module);

	if(hFile == INVALID_HANDLE_VALUE)
		return NULL;

	unsigned int fSize;

	if(GetFileAttributes(module) & FILE_ATTRIBUTE_COMPRESSED)
		fSize = GetCompressedFileSize(module, NULL);
	else
		fSize = GetFileSize(hFile, NULL);

	unsigned char *dllBin = new unsigned char[fSize];
	unsigned int nBytes;

	ReadFile(hFile, dllBin, fSize, (LPDWORD)&nBytes, FALSE);
	CloseHandle(hFile);

	//	Every PE file contains a little DOS stub for backwards compatibility
	//	it's only real relevance is that it contains a pointer to the actual
	//	PE header.
	dosHd = MakePtr(IMAGE_DOS_HEADER *, dllBin, 0);

	//	Make sure we got a valid DOS header
	if(dosHd->e_magic != IMAGE_DOS_SIGNATURE)
	{
		delete dllBin;
		return NULL;
	}

	//	Get the real PE header from the DOS stub header
	ntHd = MakePtr(IMAGE_NT_HEADERS *, dllBin, dosHd->e_lfanew);

	//	Verify the PE header
	if(ntHd->Signature != IMAGE_NT_SIGNATURE)
	{
		delete dllBin;
		return NULL;
	}

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pId);

	if(!hProcess)
		return NULL;

	//	Allocate space for the module in the remote process
	void *moduleBase = VirtualAllocEx(hProcess, 
		NULL, 
		ntHd->OptionalHeader.SizeOfImage,
		MEM_COMMIT | MEM_RESERVE,
		PAGE_EXECUTE_READWRITE);

	//	Make sure we got the memory space we wanted
	if(!moduleBase)
		return NULL;

	//	Allocate space for our stub
	void *stubBase = VirtualAllocEx(hProcess, 
		NULL,
		MakeDelta(SIZE_T, DC_stubend, DllCall_stub),
		MEM_COMMIT | MEM_RESERVE,
		PAGE_EXECUTE_READWRITE);

	//	Make sure we got the memory space we wanted
	if(!stubBase)
		return NULL;

	//	Fix up the import table of the new module
	IMAGE_IMPORT_DESCRIPTOR *impDesc = (IMAGE_IMPORT_DESCRIPTOR *)GetPtrFromRVA(
		(DWORD)(ntHd->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress), 
		ntHd, 
		(PBYTE)dllBin);

	if(ntHd->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size)
		FixImports(pId, 
			(unsigned char *)dllBin,
			ntHd,
			impDesc);
	
	//	Fix "base relocations" of the new module.  Base relocations are places
	//	in the module that use absolute addresses to reference data.  Since
	//	the base address of the module can be different at different times,
	//	the base relocation data is necessary to make the module loadable
	//	at any address.
	IMAGE_BASE_RELOCATION *reloc = (IMAGE_BASE_RELOCATION *)GetPtrFromRVA(
		(DWORD)(ntHd->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress),
		ntHd, 
		(PBYTE)dllBin);

	if(ntHd->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size)
		FixRelocs(dllBin,
			moduleBase,
			ntHd,
			reloc,
			ntHd->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size);

	//	Write the PE header into the remote process's memory space
	WriteProcessMemory(hProcess, 
		moduleBase, 
		dllBin,
		ntHd->FileHeader.SizeOfOptionalHeader + sizeof(ntHd->FileHeader) + sizeof(ntHd->Signature),
		(SIZE_T *)&nBytes);

	//	Map the sections into the remote process(they need to be aligned
	//	along their virtual addresses)
	MapSections(hProcess, moduleBase, dllBin, ntHd);
	
	//	Change the page protection on the DllCall_stub function from PAGE_EXECUTE_READ
	//	to PAGE_EXECUTE_READWRITE, so we can patch it.
	VirtualProtect((LPVOID)DllCall_stub, 
		MakeDelta(SIZE_T, DC_stubend, DllCall_stub), 
		PAGE_EXECUTE_READWRITE, 
		(DWORD *)&nBytes);

	//	Patch the stub so it calls the correct address
	*MakePtr(unsigned long *, DllCall_stub, 9) = 
		MakePtr(unsigned long, moduleBase, ntHd->OptionalHeader.AddressOfEntryPoint);


	//	Write the stub into the remote process
	WriteProcessMemory(hProcess, 
		stubBase, 
		(LPVOID)DllCall_stub, 
		MakeDelta(SIZE_T, DC_stubend, DllCall_stub),
		(SIZE_T *)&nBytes);

	//	Execute our stub in the remote process
	CreateRemoteThread(hProcess, 
		NULL, 
		0, 
		(LPTHREAD_START_ROUTINE)stubBase,
		moduleBase,		//	Pass the base address of the module as the argument to the stub.
						//	All a module handle is, is the base address of the module(except
						//	in windows CE), so we're really passing a handle to the module
						//	so that it can refer to itself, create dialogs, etc..
		0, 
		NULL);

	//	Cleanup after ourselves
	VirtualFreeEx(hProcess, stubBase, MakeDelta(SIZE_T, DC_stubend, DllCall_stub), MEM_DECOMMIT | MEM_RELEASE);
	CloseHandle(hProcess);
	delete dllBin;
	
	return (HMODULE)moduleBase;
}

//	Opens a handle to a dll file, and checks it to see if it's in any of the 
//	directories in the PATH environment variable.
HANDLE OpenModule(char *module)
{
	char pathVar[MAX_ENV_LEN], *p, *p2;
	char modPath[MAX_PATH] = {0};
	char c;

	HANDLE hFile;

	
	//	If it's an absolute path that was passed to us(i.e. C:\xxx or D:\xxx) then we don't need
	//	do scan the PATH folders.
	if(!strchr(module, ':'))
	{
		GetEnvironmentVariable("PATH", pathVar, MAX_ENV_LEN);

		p = pathVar;
		while(p2 = strchr(p, ';'))
		{
			memset(modPath, 0, sizeof(modPath));

			c = *p2;
			*p2 = '\0';
			strncpy(modPath, p, MAX_PATH);
			strcat(modPath, "\\");
			strcat(modPath, module);	
			p = ++p2;

			hFile = CreateFile(modPath, 
				GENERIC_READ, 
				FILE_SHARE_READ | FILE_SHARE_WRITE, 
				NULL, 
				OPEN_EXISTING, 
				FILE_ATTRIBUTE_NORMAL, 
				NULL);
			
			if(hFile && hFile != INVALID_HANDLE_VALUE)
				return hFile;
		}	
	}

	hFile = CreateFile(module, 
		GENERIC_READ, 
		FILE_SHARE_READ | FILE_SHARE_WRITE, 
		NULL, 
		OPEN_EXISTING, 
		FILE_ATTRIBUTE_NORMAL, 
		NULL);

    return hFile;
}

bool MapSections(HANDLE hProcess, void *moduleBase, void *dllBin, IMAGE_NT_HEADERS *ntHd)
{
	IMAGE_SECTION_HEADER *header = IMAGE_FIRST_SECTION(ntHd);
	unsigned int nBytes = 0;
	unsigned int virtualSize = 0;
	unsigned int n = 0;

	//	Loop through the list of sections
	for(unsigned int i = 0; ntHd->FileHeader.NumberOfSections; i++)
	{
		//	Once we've reached the SizeOfImage, the rest of the sections
		//	don't need to be mapped, if there are any.
		if(nBytes >= ntHd->OptionalHeader.SizeOfImage)
			break;
		
		WriteProcessMemory(hProcess, 
			MakePtr(LPVOID, moduleBase, header->VirtualAddress), 
			MakePtr(LPCVOID, dllBin, header->PointerToRawData), 
			header->SizeOfRawData,
			(LPDWORD)&n);

		virtualSize = header->VirtualAddress;
		header++;
		virtualSize = header->VirtualAddress - virtualSize;
		nBytes += virtualSize;

		//	Set the proper page protections for this section.
		//	This really could be skipped, but it's not that
		//	hard to implement and it makes it more like a
		//	real loader.
		VirtualProtectEx(hProcess, 
			MakePtr(LPVOID, moduleBase, header->VirtualAddress),
			virtualSize,
			header->Characteristics & 0x00FFFFFF,
			NULL);
	}

	return true;
}

bool FixImports(unsigned long pId, void *base, IMAGE_NT_HEADERS *ntHd, IMAGE_IMPORT_DESCRIPTOR *impDesc)
{
	char *module;

	//	Loop through all the required modules
	while((module = (char *)GetPtrFromRVA((DWORD)(impDesc->Name), ntHd, (PBYTE)base)))
	{
		//	If the library is already loaded(like kernel32.dll or ntdll.dll) LoadLibrary will
		//	just return the handle to that module.
        HMODULE localMod = LoadLibrary(module);

		//	If the module isn't loaded in the remote process, we recursively call the
		//	module mapping code.  This has the added benefit of ensuring that any of
		//	the current module's dependencies will be just as invisble as this one.
		if(!GetRemoteModuleHandle(pId, module))
		{
			MODULE mod;
			strncpy(mod.name, module, sizeof(mod.name));
			mod.handle = MapRemoteModule(pId, module);
			
			mappedMods.push_back(mod);
		}
        
		//	Lookup the first import thunk for this module
		//	NOTE: It is possible this module could forward functions...which is something
		//	that I really should handle.  Maybe i'll add support for forwared functions
		//	a little bit later.
		IMAGE_THUNK_DATA *itd = 
			(IMAGE_THUNK_DATA *)GetPtrFromRVA((DWORD)(impDesc->FirstThunk), ntHd, (PBYTE)base);

		while(itd->u1.AddressOfData)
		{
			IMAGE_IMPORT_BY_NAME *iibn;
			iibn = (IMAGE_IMPORT_BY_NAME *)GetPtrFromRVA((DWORD)(itd->u1.AddressOfData), ntHd, (PBYTE)base);

             itd->u1.Function = MakePtr(DWORD, GetRemoteProcAddress(pId, 
				module, 
				(char *)iibn->Name), 0);

			itd++;
		}        
		impDesc++;
	}
	
	return true;
}

bool FixRelocs(void *base, void *rBase, IMAGE_NT_HEADERS *ntHd, IMAGE_BASE_RELOCATION *reloc, unsigned int size)
{
	unsigned long ImageBase = ntHd->OptionalHeader.ImageBase;
	unsigned int nBytes = 0;

	unsigned long delta = MakeDelta(unsigned long, rBase, ImageBase);

	while(1)
	{
		unsigned long *locBase = 
			(unsigned long *)GetPtrFromRVA((DWORD)(reloc->VirtualAddress), ntHd, (PBYTE)base);
		unsigned int numRelocs = (reloc->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);

		if(nBytes >= size) break;

		unsigned short *locData = MakePtr(unsigned short *, reloc, sizeof(IMAGE_BASE_RELOCATION));
		for(unsigned int i = 0; i < numRelocs; i++)
		{        
			if(((*locData >> 12) & IMAGE_REL_BASED_HIGHLOW))
			    *MakePtr(unsigned long *, locBase, (*locData & 0x0FFF)) += delta;

			locData++;
		}

		nBytes += reloc->SizeOfBlock;
		reloc = (IMAGE_BASE_RELOCATION *)locData;
	}

	return true;
}


FARPROC GetRemoteProcAddress(unsigned long pId, char *module, char *func)
{
	HMODULE remoteMod = GetRemoteModuleHandle(pId, module);
	HMODULE localMod = GetModuleHandle(module);

	//	Account for potential differences in base address
	//	of modules in different processes.
	unsigned long delta = MakeDelta(unsigned long, remoteMod, localMod);
	return MakePtr(FARPROC, GetProcAddress(localMod, func), delta);
}

unsigned long GetProcessIdByName(char *process)
{
   PROCESSENTRY32 pe;
   HANDLE thSnapshot;
   BOOL retval, ProcFound = false;

   thSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

   if(thSnapshot == INVALID_HANDLE_VALUE)
   {
      MessageBox(NULL, "Error: unable to create toolhelp snapshot", "Loader", NULL);
      return false;
   }

   pe.dwSize = sizeof(PROCESSENTRY32);

    retval = Process32First(thSnapshot, &pe);

   while(retval)
   {
      if(StrStrI(pe.szExeFile, process) )
      {
         ProcFound = true;
         break;
      }

      retval    = Process32Next(thSnapshot,&pe);
      pe.dwSize = sizeof(PROCESSENTRY32);
   }

   return pe.th32ProcessID;
}

HMODULE GetRemoteModuleHandle(unsigned long pId, char *module)
{
	MODULEENTRY32 modEntry;
	HANDLE tlh = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pId);

	modEntry.dwSize = sizeof(MODULEENTRY32);
    Module32First(tlh, &modEntry);

	do
	{
		if(!stricmp(modEntry.szModule, module))
			return modEntry.hModule;
		modEntry.dwSize = sizeof(MODULEENTRY32);
	}
	while(Module32Next(tlh, &modEntry));

	//	If we didn't find it in the real list, check to see if it was one
	//	of the dependencies that we just loaded.
	for(unsigned int i = 0; i < mappedMods.size(); i++)
		if(!stricmp(mappedMods[i].name, module))
			return mappedMods[i].handle;

	return NULL;
}

//	Matt Pietrek's function
PIMAGE_SECTION_HEADER GetEnclosingSectionHeader(DWORD rva, PIMAGE_NT_HEADERS pNTHeader) 
{
    PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(pNTHeader);
    unsigned int i;
    
    for ( i = 0; i < pNTHeader->FileHeader.NumberOfSections; i++, section++ )
    {
		// This 3 line idiocy is because Watcom's linker actually sets the
		// Misc.VirtualSize field to 0.  (!!! - Retards....!!!)
		DWORD size = section->Misc.VirtualSize;
		if ( 0 == size )
			size = section->SizeOfRawData;
			
        // Is the RVA within this section?
        if ( (rva >= section->VirtualAddress) && 
             (rva < (section->VirtualAddress + size)))
            return section;
    }
    
    return 0;
}

//	This function is also Pietrek's
LPVOID GetPtrFromRVA( DWORD rva, IMAGE_NT_HEADERS *pNTHeader, PBYTE imageBase )
{
	PIMAGE_SECTION_HEADER pSectionHdr;
	INT delta;
		
	pSectionHdr = GetEnclosingSectionHeader( rva, pNTHeader );
	if ( !pSectionHdr )
		return 0;
 
	delta = (INT)(pSectionHdr->VirtualAddress-pSectionHdr->PointerToRawData);
	return (PVOID) ( imageBase + rva - delta );
}