#include <tchar.h>
#include "main.h"

//原函数类型定义
typedef int (WINAPI* MsgBoxW)(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType);
typedef HWND(WINAPI* PCreateMutexW)(LPSECURITY_ATTRIBUTES lpMutexAttributes,BOOL bInitialOwner, LPCTSTR lpName);

MsgBoxW OldMsgBoxW=NULL;//指向原函数的指针
FARPROC pfOldMsgBoxW;  //指向函数的远指针

PCreateMutexW OldCreateMutexW=NULL;//指向原函数的指针
FARPROC pfOldCreateMutexW;  //指向函数的远指针

BYTE OldCode[5]; //原系统API入口代码
BYTE NewCode[5]; //原系统API新的入口代码(jmp xxxxxxxx)

HANDLE hProcess = NULL;//本程序进程句柄
HINSTANCE hInst = NULL;//API所在的dll文件句柄

void HookOn();
void HookOff();
int WINAPI MyMessageBoxW(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType)
{
    HookOff();//调用原函数之前，记得先恢复HOOK呀，不然是调用不到的
    //如果不恢复HOOK，就调用原函数，会造成死循环
    //毕竟调用的还是我们的函数，从而造成堆栈溢出，程序崩溃。

    int nRet = ::MessageBoxW(hWnd, L"MessageBoxW is HOOK", lpCaption, uType);

    HookOn();//调用完原函数后，记得继续开启HOOK，不然下次会HOOK不到。

    return nRet;
}
HWND WINAPI MyCreateMutexW(LPCTSTR lpName, LPCTSTR lpCName)
{
	HWND hHandle = 0;
    //TRACE(lpText);
	HookOff();//调用原函数之前，记得先恢复HOOK呀，不然是调用不到的
			  //如果不恢复HOOK，就调用原函数，会造成死循环
			  //毕竟调用的还是我们的函数，从而造成堆栈溢出，程序崩溃。
    ::MessageBoxW(NULL,L"haha,CreateMutexW is HOOK",L"Message",MB_OK);
	hHandle=::FindWindowW(NULL, L"WECHAT.EXE_NEW");

	HookOn();//调用完原函数后，记得继续开启HOOK，不然下次会HOOK不到。

	return hHandle;
}


//开启钩子的函数
void HookOn()
{
     if ( NULL == hProcess)
    {
        return;
    }

    DWORD dwTemp=0;
    DWORD dwOldProtect;
    //修改API函数入口前5个字节为jmp xxxxxx
    //VirtualProtectEx(hProcess,(PVOID)pfOldMsgBoxW,5,PAGE_READWRITE,&dwOldProtect);
    //WriteProcessMemory(hProcess,(LPVOID)pfOldMsgBoxW,NewCode,5,0);
    //VirtualProtectEx(hProcess,(PVOID)pfOldMsgBoxW,5,dwOldProtect,&dwTemp);

    //修改API函数入口前5个字节为jmp xxxxxx
    VirtualProtectEx(hProcess,(PVOID)pfOldCreateMutexW,5,PAGE_READWRITE,&dwOldProtect);
    WriteProcessMemory(hProcess,(LPVOID)pfOldCreateMutexW,NewCode,5,0);
    VirtualProtectEx(hProcess,(PVOID)pfOldCreateMutexW,5,dwOldProtect,&dwTemp);
}

//关闭钩子的函数
void HookOff()
{
    if ( NULL == hProcess)
    {
        return;
    }

    DWORD dwTemp=0;
    DWORD dwOldProtect;

     //恢复API函数入口前5个字节
 //VirtualProtectEx(hProcess,(PVOID)pfOldMsgBoxW,5,PAGE_READWRITE,&dwOldProtect);
 //WriteProcessMemory(hProcess,(LPVOID)pfOldMsgBoxW,OldCode,5,0);
 //VirtualProtectEx(hProcess,(PVOID)pfOldMsgBoxW,5,dwOldProtect,&dwTemp);

  //恢复API函数入口前5个字节
 VirtualProtectEx(hProcess,(PVOID)pfOldCreateMutexW,5,PAGE_READWRITE,&dwOldProtect);
 WriteProcessMemory(hProcess,(LPVOID)pfOldCreateMutexW,OldCode,5,0);
 VirtualProtectEx(hProcess,(PVOID)pfOldCreateMutexW,5,dwOldProtect,&dwTemp);
}

//获取API函数入口前个字节
//旧入口前个字节保存在前面定义的字节数组BYTE OldCode[5]
//新入口前个字节保存在前面定义的字节数组BYTE NewCode[5]
void GetApiEntrance()
{
    //获取原API入口地址
    HMODULE hmod = ::LoadLibrary( _T("User32.dll" ));
    OldMsgBoxW = (MsgBoxW)::GetProcAddress(hmod, "MessageBoxW");
    pfOldMsgBoxW = (FARPROC)OldMsgBoxW;

    if (NULL == pfOldMsgBoxW)
    {
        ::MessageBox(NULL, _T("get APIentry error"), _T("error!"), 0);
        return;
    }
    hmod = ::LoadLibrary(_T("Kernel32.dll"));
    OldCreateMutexW=(PCreateMutexW)::GetProcAddress(hmod,"CreateMutexW");
    pfOldCreateMutexW=(FARPROC)OldCreateMutexW;

    if (pfOldCreateMutexW==NULL)
    {
        MessageBox(NULL,_T("获取原API入口地址出错"),_T("error!"),0);
        return;
    }
  // 将原API的入口前5个字节代码保存到OldCode[]
  // 将原API的入口前5个字节代码保存到OldCode[]
	_asm
	{
		lea edi, OldCode		//获取OldCode数组的地址,放到edi
		mov esi, pfOldCreateMutexW //获取原API入口地址，放到esi
		cld	  //方向标志位，为以下两条指令做准备
		movsd //复制原API入口前4个字节到OldCode数组
		movsb //复制原API入口第5个字节到OldCode数组
	}

    NewCode[0]=0xe9;//实际上xe9就相当于jmp指令

    //获取MyMessageBoxW的相对地址,为Jmp做准备
    //int nAddr= UserFunAddr ?SysFunAddr - （我们定制的这条指令的大小）;
    //Jmp nAddr;
    //（我们定制的这条指令的大小）, 这里是，个字节嘛
	_asm
	{
		lea eax, MyCreateMutexW //获取我们的MyMessageBoxW函数地址
		mov ebx, pfOldCreateMutexW  //原系统API函数地址
		sub eax, ebx			 //int nAddr= UserFunAddr – SysFunAddr
		sub eax, 5			 //nAddr=nAddr-5
		mov dword ptr[NewCode + 1], eax //将算出的地址nAddr保存到NewCode后面4个字节
										//注：一个函数地址占4个字节
	}

    //填充完毕，现在NewCode[]里的指令相当于Jmp MyMessageBoxW
    //既然已经获取到了Jmp MyMessageBoxW
    //现在该是将Jmp MyMessageBoxW写入原API入口前个字节的时候了
    //知道为什么是个字节吗？
    //Jmp指令相当于xe9,占一个字节的内存空间
    //MyMessageBoxW是一个地址，其实是一个整数，占个字节的内存空间
    //int n=0x123;   n占个字节和MyMessageBoxW占个字节是一样的
    //1+4=5，知道为什么是个字节了吧
    HookOn();
}

// a sample exported function
void DLL_EXPORT SomeFunction(const LPCSTR sometext)
{
    MessageBoxA(0, sometext, "DLL Message", MB_OK | MB_ICONINFORMATION);
}

extern "C" DLL_EXPORT BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            // attach to process
            // return FALSE to fail DLL load
            {
				MessageBox(NULL, _T("注入成功"), _T("提示"), MB_ICONINFORMATION);
                DWORD dwPid=::GetCurrentProcessId();
                hProcess=OpenProcess(PROCESS_ALL_ACCESS,0,dwPid);
                GetApiEntrance();
            }
            break;

		case DLL_PROCESS_DETACH:
			// detach from process
			{
				MessageBox(NULL, _T("反注入成功"), _T("提示"), MB_ICONINFORMATION);
				HookOff();
			}
            break;

        case DLL_THREAD_ATTACH:
            // attach to thread
            break;

        case DLL_THREAD_DETACH:
            // detach from thread
            break;
    }
    return TRUE; // succesful
}
