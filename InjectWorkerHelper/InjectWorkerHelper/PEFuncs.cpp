#include "stdafx.h"
#include "PEFuncs.h"

BOOL LoadFileR(LPCTSTR lpFilename,PMAP_FILE_STRUCT pstMapFile)
{

	HANDLE hFile = NULL;
	HANDLE hMapping = NULL;
	LPVOID ImageBase = NULL;

	memset(pstMapFile,0,sizeof(MAP_FILE_STRUCT));

	hFile = CreateFile(lpFilename, GENERIC_READ | STANDARD_RIGHTS_ALL, FILE_SHARE_READ, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,0);
	
	if (!hFile)				   
		return FALSE;

	hMapping=CreateFileMapping(hFile,NULL,PAGE_READONLY,0,0,NULL);
	if(!hMapping)
	{									
		CloseHandle(hFile);
		return FALSE;
	}
	ImageBase=MapViewOfFile(hMapping,FILE_MAP_READ,0,0,0);
    if(!ImageBase)
	{									
		CloseHandle(hMapping);
		CloseHandle(hFile);
		return FALSE;
	}
	pstMapFile->hFile=hFile;
	pstMapFile->hMapping=hMapping;
	pstMapFile->ImageBase=ImageBase;
	return TRUE;
}

void UnLoadFile(PMAP_FILE_STRUCT pstMapFile)
{
	if(pstMapFile->ImageBase)
		UnmapViewOfFile(pstMapFile->ImageBase);
	
	if(pstMapFile->hMapping)
		CloseHandle(pstMapFile->hMapping);
	
	if(pstMapFile->hFile)
		CloseHandle(pstMapFile->hFile);
	
}

BOOL IsPEFile(LPVOID ImageBase)
{
    PIMAGE_DOS_HEADER  pDH=NULL;
    PIMAGE_NT_HEADERS  pNtH=NULL;
  
    if(!ImageBase)
	  return FALSE;
   
    pDH=(PIMAGE_DOS_HEADER)ImageBase;
    if(pDH->e_magic!=IMAGE_DOS_SIGNATURE)
         return FALSE;


#ifdef _WIN64
	pNtH = (PIMAGE_NT_HEADERS)((DWORD)pDH + pDH->e_lfanew);
#else 
	pNtH = (PIMAGE_NT_HEADERS32)((DWORD)pDH + pDH->e_lfanew);
#endif



    if (pNtH->Signature != IMAGE_NT_SIGNATURE )
        return FALSE;

    return TRUE;
	
}
//
PIMAGE_NT_HEADERS  GetNtHeaders(LPVOID ImageBase)
{
    
	if(!IsPEFile(ImageBase))
		return NULL;
	PIMAGE_NT_HEADERS  pNtH = NULL;
	PIMAGE_DOS_HEADER  pDH = NULL;
	pDH=(PIMAGE_DOS_HEADER)ImageBase;
	pNtH=(PIMAGE_NT_HEADERS)((DWORD)pDH+pDH->e_lfanew);

	return pNtH;

}

//
PIMAGE_FILE_HEADER   GetFileHeader(LPVOID ImageBase)
{
    PIMAGE_DOS_HEADER  pDH=NULL;
    PIMAGE_NT_HEADERS  pNtH=NULL;
    PIMAGE_FILE_HEADER pFH=NULL;
    
    if(!IsPEFile(ImageBase))
		return NULL;
    pDH=(PIMAGE_DOS_HEADER)ImageBase;
    pNtH=(PIMAGE_NT_HEADERS)((DWORD)pDH+pDH->e_lfanew);
    pFH=&pNtH->FileHeader;
    return pFH;
}
PIMAGE_OPTIONAL_HEADER GetOptionalHeader(LPVOID ImageBase)
{
    PIMAGE_DOS_HEADER  pDH=NULL;
    PIMAGE_NT_HEADERS  pNtH=NULL;
    PIMAGE_OPTIONAL_HEADER pOH=NULL;
   
     if(!IsPEFile(ImageBase))
		return NULL;
    pDH=(PIMAGE_DOS_HEADER)ImageBase;
    pNtH=(PIMAGE_NT_HEADERS)((DWORD)pDH+pDH->e_lfanew);
    pOH=&pNtH->OptionalHeader;
    return pOH;
}
PIMAGE_SECTION_HEADER GetFirstSectionHeader(LPVOID ImageBase)
{
  	PIMAGE_NT_HEADERS     pNtH=NULL;
    PIMAGE_SECTION_HEADER pSH=NULL;
    
    pNtH=GetNtHeaders(ImageBase);
   	pSH=IMAGE_FIRST_SECTION(pNtH);
 	return  pSH;
}

LPVOID MyRvaToPtr(PIMAGE_NT_HEADERS pNtH,void* ImageBase,unsigned long dwRVA)
{	
 	return ImageRvaToVa(pNtH,ImageBase,dwRVA,NULL);
		
}

LPVOID GetDirectoryEntryToData(LPVOID ImageBase,USHORT DirectoryEntry)
{
	DWORD dwDataStartRVA = 0;
	LPVOID pDirData=NULL;
	PIMAGE_NT_HEADERS     pNtH=NULL;
	PIMAGE_OPTIONAL_HEADER pOH=NULL;

	pNtH=GetNtHeaders(ImageBase);
	if(!pNtH)
		return NULL;
	pOH=GetOptionalHeader(ImageBase);
	if(!pOH)
		return NULL;
    dwDataStartRVA=pOH->DataDirectory[DirectoryEntry].VirtualAddress;
      if(!dwDataStartRVA)
        return NULL;
  
	pDirData=MyRvaToPtr(pNtH,ImageBase,dwDataStartRVA);
   if(!pDirData)
		return NULL;	 
   	return  pDirData;
}


DWORD GetDirectorySize(LPVOID ImageBase, USHORT DirectoryEntry)
{
	DWORD dwSize = 0;
	LPVOID pDirData = NULL;
	PIMAGE_NT_HEADERS     pNtH = NULL;
	PIMAGE_OPTIONAL_HEADER pOH = NULL;

	pNtH = GetNtHeaders(ImageBase);
	if (!pNtH)
		return NULL;
	pOH = GetOptionalHeader(ImageBase);
	if (!pOH)
		return NULL;
	dwSize = pOH->DataDirectory[DirectoryEntry].Size;

	return  dwSize;
}
PIMAGE_EXPORT_DIRECTORY  GetExportDirectory(LPVOID ImageBase)
{
   
	PIMAGE_EXPORT_DIRECTORY pExportDir=NULL;
	pExportDir=(PIMAGE_EXPORT_DIRECTORY)GetDirectoryEntryToData(ImageBase,IMAGE_DIRECTORY_ENTRY_EXPORT);
    if(!pExportDir)
		return NULL;	 
   	return  pExportDir;
}

PIMAGE_BASE_RELOCATION GetFirstRelocation(LPVOID ImageBase)
{
	PIMAGE_BASE_RELOCATION pImageBase = NULL;
	pImageBase = (PIMAGE_BASE_RELOCATION)GetDirectoryEntryToData(ImageBase, IMAGE_DIRECTORY_ENTRY_BASERELOC);

	if (!pImageBase)
	{
		return NULL;
	}

	return pImageBase;
}

PIMAGE_BASE_RELOCATION GetNextRelocation(int nSum, LPVOID ImageBase)
{
	PIMAGE_BASE_RELOCATION pImageBase = NULL;
	DWORD dwNewAddr = NULL;
	dwNewAddr = (DWORD)GetDirectoryEntryToData(ImageBase, IMAGE_DIRECTORY_ENTRY_BASERELOC);

	pImageBase = (PIMAGE_BASE_RELOCATION)(dwNewAddr + nSum);
	if (!pImageBase)
	{
		return NULL;
	}

	return pImageBase;
}

PIMAGE_RESOURCE_DIRECTORY GetFirstResDirectory(LPVOID ImageBase)
{
	PIMAGE_RESOURCE_DIRECTORY pImageBase = NULL;
	pImageBase = (PIMAGE_RESOURCE_DIRECTORY)GetDirectoryEntryToData(ImageBase, IMAGE_DIRECTORY_ENTRY_RESOURCE);

	if (!pImageBase)
	{
		return NULL;
	}

	return pImageBase;


}
		


PIMAGE_IMPORT_DESCRIPTOR  GetFirstImportDesc(LPVOID ImageBase)
{
	PIMAGE_IMPORT_DESCRIPTOR pImportDesc;
	pImportDesc=(PIMAGE_IMPORT_DESCRIPTOR)GetDirectoryEntryToData(ImageBase,IMAGE_DIRECTORY_ENTRY_IMPORT);
    if(!pImportDesc)
		return NULL;	 
   	return  pImportDesc;
}
DWORD   GetNumOfExportFuncs(LPVOID ImageBase,PIMAGE_EXPORT_DIRECTORY pExportDir)
{
	DWORD   dwnum=0;
    PDWORD pdwRvas=NULL;
	
/*	if(!IsPEFile(ImageBase))
		return NULL;
*/
 PIMAGE_NT_HEADERS pNtH=GetNtHeaders(ImageBase);
 
  pdwRvas=(PDWORD)MyRvaToPtr(pNtH,ImageBase,pExportDir->AddressOfFunctions);
	for(DWORD i=0;i<pExportDir->NumberOfFunctions;i++)
	{	
		if(*pdwRvas)
			++dwnum;
		
		++pdwRvas;
		
	} 
	return dwnum;
}

BOOL  IsDataDirPresent(LPVOID ImageBase,USHORT DirectoryEntry)
{

    if(!GetDirectoryEntryToData(ImageBase,DirectoryEntry))
		return FALSE;
	return TRUE;

}

PIMAGE_DOS_HEADER GetDosHeader(LPVOID ImageBase)
{
	PIMAGE_DOS_HEADER  pDH = NULL;

	if (!IsPEFile(ImageBase))
	{
		return NULL;
	}
		
	pDH = (PIMAGE_DOS_HEADER)ImageBase;
	return pDH;
}