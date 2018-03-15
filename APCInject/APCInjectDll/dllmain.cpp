// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	//MessageBox(NULL, ("Enter Inject!"), ("Message"), MB_ICONINFORMATION);
	char czTime[MAXCHAR] = { 0 };
	if (sizeof(time_t) > 4)
	{
		sprintf(czTime, "D:\\inject_success_%lld.log", time(0));
	}
	else
	{
		sprintf(czTime, "D:\\inject_success_%ld.log", time(0));
	}
	FILE * pFile = fopen(czTime, "wb");
	if (pFile)
	{
		fprintf(pFile, "注入已成功！进入注入模式。\r\n");
		fclose(pFile);
	}
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

