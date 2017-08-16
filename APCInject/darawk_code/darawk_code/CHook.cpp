#include "CHook.h"

//	Initialize static members
/////////////////////////
bool CHook::debugInitialized = false;
DebugHooks CHook::debugHooks;
CHook *CHook::threadHook;
map<FARPROC, FARPROC> CHook::int3hooks;


//	Constructor
CHook::CHook()
{
	m_hookFrom = NULL;
	m_hookTo = NULL;
	m_target = NULL;
	memset(entryStub, 0, sizeof(entryStub));
	bHooked = false;
	stubLen = 0;

	if(!debugInitialized)
	{
		AddVectoredExceptionHandler(0, (PVECTORED_EXCEPTION_HANDLER)Int3HookHandler);
		AddVectoredExceptionHandler(0, (PVECTORED_EXCEPTION_HANDLER)DebugHookHandler);
		debugInitialized = true;
		threadHook = new CHook;
		threadHook->Hook(GetProcAddress(GetModuleHandle("ntdll.dll"), "CsrNewThread"), (FARPROC)ThreadCallback);
	}
}

//	Destructor
CHook::~CHook()
{
	//	Remove any hooks we may have
	Unhook();
}

//	Hook a function
bool CHook::Hook(FARPROC hookFrom, FARPROC hookTo, CallingConventions cconv)
{
	//	Make sure we got valid function pointers, and that a function
	//	isn't currently being hooked by this instance of CHook.
	if(IsBadReadPtr(hookFrom, 5) || IsBadReadPtr(hookTo, 1) || bHooked)
		return false;

	m_hookFrom = hookFrom;
	m_hookTo = hookTo;

	//	Save the first 5 bytes, which could get over-written
	memcpy(oldbytes, m_hookFrom, 5);

	if(BuildStub())
		WriteJump(m_hookFrom, m_hookTo);
	else
	{
		WriteInt3(m_hookFrom);
		int3hooks[m_hookFrom] = m_hookTo;
	}

	//	Store all the necessary data about this hook
	memcpy(patchbytes, m_hookFrom, 5);
	convention = cconv;
	bHooked = true;
	type = Patch;
	bEnabled = true;
	return true;
}

bool CHook::DebugHook(FARPROC hookFrom, FARPROC hookTo, CallingConventions cconv)
{
	if(bHooked)
		return false;

	s32 i = SetDebugBreak(hookFrom);

	if(i == -1)
		return false;

	debugHooks.targetFuncs[i] = hookFrom;
	debugHooks.hookFuncs[i] = hookTo;

	//	Build a call stub
	u32 len = mlde32(MakePtr(void *, hookFrom, 0));

	memset(entryStub, 0, sizeof(entryStub));
	memcpy(entryStub, MakePtr(void *, hookFrom, 0), len);

	u8 relJmp[] = {INSTR_RELJMP, 0x00, 0x00, 0x00, 0x00};
	*(u32 *)(relJmp + 1) = MakeDelta(u32, hookFrom, entryStub) - 5;
	memcpy(&entryStub[len], relJmp, sizeof(relJmp));

	stubLen = len + sizeof(relJmp);
	m_hookFrom = hookFrom;
	m_hookTo = hookTo;
	convention = cconv;
	bHooked = true;

    return true;
}

//	Hook the IAT of a specific module(this includes the .exe file)
bool CHook::HookIAT(FARPROC hookFrom, FARPROC hookTo, HMODULE target, CallingConventions cconv)
{

	if(bHooked)
		return false;

	dosHd = (IMAGE_DOS_HEADER *)target;
	ntHd = MakePtr(IMAGE_NT_HEADERS *, target, dosHd->e_lfanew);

	IMAGE_IMPORT_DESCRIPTOR *impDesc = (IMAGE_IMPORT_DESCRIPTOR *)GetPtrFromRVA(ntHd->OptionalHeader.DataDirectory[IMAGE_IMPORT].VirtualAddress, true);

	if(!(impDesc->Name))
		return false;

	IMAGE_THUNK_DATA *thunk = (IMAGE_THUNK_DATA *)GetPtrFromRVA(impDesc->FirstThunk, true);
		
	while(impDesc->Name)
	{
		thunk = MakePtr(IMAGE_THUNK_DATA *, target, impDesc->FirstThunk);

		while(thunk->u1.Function && thunk->u1.Function != MakePtr(u32, hookFrom, 0))
			thunk++;

		if(thunk->u1.Function == MakePtr(u32, hookFrom, 0))
			break;

		impDesc++;
	}

	if(impDesc->Name && thunk->u1.Function)
	{
		u32 oldprot, oldprot2;
		VirtualProtect(&(thunk->u1.Function), sizeof(u32), PAGE_READWRITE, (DWORD *)&oldprot);
		thunk->u1.Function = MakePtr(u32, hookTo, 0);
		VirtualProtect(&(thunk->u1.Function), sizeof(u32), oldprot, (DWORD *)&oldprot2);
	}
	else
		return false;

	convention = cconv;
	m_hookFrom = hookFrom;
	m_hookTo = hookTo;
	m_target = target;
	bHooked = true;
	type = IAT;
	return true;
}


bool CHook::HookEAT(FARPROC hookFrom, FARPROC hookTo, CallingConventions cconv)
{
    if(bHooked)
		return false;

	HMODULE target;
	if(!GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCSTR)hookFrom, &target))
		return false;


	dosHd = (IMAGE_DOS_HEADER *)target;
	ntHd = MakePtr(IMAGE_NT_HEADERS *, target, dosHd->e_lfanew);
	IMAGE_EXPORT_DIRECTORY *ied = (IMAGE_EXPORT_DIRECTORY *)GetPtrFromRVA(ntHd->OptionalHeader.DataDirectory[IMAGE_EXPORT].VirtualAddress, true);

	FARPROC *funcs = (FARPROC *)GetPtrFromRVA(ied->AddressOfFunctions, true);

	u32 i;
	for(i = 0; i < ied->NumberOfFunctions; i++)
		if(MakePtr(FARPROC, target, funcs[i]) == hookFrom)
			break;

	if(i >= ied->NumberOfFunctions)
		return false;

	u32 oldprot, oldprot2;
	VirtualProtect(&funcs[i], sizeof(u32), PAGE_READWRITE, (DWORD *)&oldprot);
	funcs[i] = MakeDelta(FARPROC, hookTo, target);
	VirtualProtect(&funcs[i], sizeof(u32), oldprot, (DWORD *)&oldprot2);

    convention = cconv;
	m_hookFrom = hookFrom;
	m_hookTo = hookTo;
	m_target = target;
	bHooked = true;
	type = EAT;
	return true;
}

//	Unhook a function
bool CHook::Unhook()
{
	//	Make sure we are actually currently hooking something before
	//	trying to unhook.
	if(!bHooked)
		return false;

    switch(type)
	{
	case Patch:
		//	Remove the relative jump from the entry point
		//	of the hooked function.
		u32 oldprot, oldprot2;
		VirtualProtect(m_hookFrom, 5, PAGE_EXECUTE_READWRITE, (DWORD *)&oldprot);
		memcpy(m_hookFrom, oldbytes, 5);
		VirtualProtect(m_hookFrom, 5, oldprot, (DWORD *)&oldprot2);

		//	Clear the code stub
		memset(entryStub, 0, sizeof(entryStub));
		memset(oldbytes, 0, sizeof(oldbytes));

		if(int3hooks[m_hookFrom])
			int3hooks.erase(m_hookFrom);

		//	This class is no longer hooking a function
		bHooked = false;
		break;
	case IAT:
		//	Essentially, just hook it all back the opposite way
		if(HookIAT(m_hookTo, m_hookFrom, m_target, convention))
			bHooked = false;
		else
			bHooked = true;
		break;
	case EAT:
		//	Same as above, just reverse it
		bHooked = false;
		if(HookEAT(m_hookTo, m_hookFrom, convention))
			m_target = NULL;
		else
			bHooked = true;
		break;
	case Debug:
		memset(entryStub, 0, sizeof(entryStub));
		stubLen = 0;

		u32 i;
		for(i = 0; debugHooks.targetFuncs[i] != m_hookFrom; i++);

        debugHooks.targetFuncs[i] = 0;
		debugHooks.hookFuncs[i] = 0;

		if(ClearDebugBreak(i))
			bHooked = false;
		break;
	default:
		//	?
		break;
	}
	
	stubLen = 0;
	if(!bHooked)
		return true;

	return false;
}

bool CHook::Enable()
{
	if(bEnabled && type != Patch)
		return false;

	u32 oldprot, oldprot2;
	VirtualProtect(m_hookFrom, 5, PAGE_EXECUTE_READWRITE, (DWORD *)&oldprot);
	memcpy(m_hookFrom, patchbytes, 5);
	VirtualProtect(m_hookFrom, 5, oldprot, (DWORD *)&oldprot2);
	bEnabled = true;
	return true;
}

bool CHook::Disable()
{
	if(!bEnabled && type != Patch)
		return false;

	u32 oldprot, oldprot2;
	VirtualProtect(m_hookFrom, 5, PAGE_EXECUTE_READWRITE, (DWORD *)&oldprot);
	memcpy(m_hookFrom, oldbytes, 5);
	VirtualProtect(m_hookFrom, 5, oldprot, (DWORD *)&oldprot2);
	bEnabled = false;
	return true;
}


u32 CHook::OriginalFunc(u32 num, ...)
{
	u32 retVal;

	if(!bHooked)
		return 0;

	u32 *argArray = NULL;
	if(num)
	{
		va_list args;
		va_start(args, num);
	
		argArray = new u32[num];

		for(u32 i = 0; i < num; i++)
			argArray[i] = va_arg(args, u32);

		//	Push the arguments onto the stack
		_asm
		{
			mov ecx, num
	loop1:
			dec ecx
			mov eax, argArray
			mov ebx, [eax + ecx * 4]
			push ebx
			test ecx, ecx
			jnz loop1
		}

		va_end(args);
	}

	FARPROC tmp;
	switch(type)
	{
	//	Debug hooks are called in exactly the same way as patch's
	case Debug:
	case Patch:
		tmp = MakePtr(FARPROC, entryStub, 0);
		break;
	//	IAT/EAT hooks are the same for this part
	case IAT:
	case EAT:
		tmp = m_hookFrom;
		break;
	default: 
		// ?
		break;
	}


	//	Call it based on the calling convention specified by the user
	switch(convention)
	{
	//	Properties of Normal:
	//		Arguments are pushed onto the stack from left to right
	//		Caller restores ESP after the function exits
	//		
	//		NOTE: Normal is not a real calling convention.  It's my
	//		combination of Stdcall and Cdecl.  It simply restores ESP
	//		regardless of whether or not the callee already did so.
	case Normal:
		_asm
		{
			mov ecx, num
			lea ecx, [ecx * 4]
			xor ebx, ebx
			add ebx, ecx
			add ebx, esp	//	Calculate the value of esp before args were pushed, and store
							//	it in ebx
			mov eax, tmp
			call eax
			mov retVal, eax
			mov esp, ebx	//	Restore esp
		}
		break;
	//	Properties of Fastcall:
	//		First two arguments are passed in ecx and edx
	//		
	case Fastcall:
		_asm
		{
			mov eax, argArray
			mov ecx, [eax]
			mov edx, [eax + 4]
			add esp, 8			//	Get rid of the first 2 args that we pushed earlier
			mov eax, tmp
			call eax
			mov retVal, eax
			add esp, 8
		}
		break;
	//	Properties of NoRet:
	//		Takes first argument where the return address should be
	//		Function does not return
	case NoRet:
		_asm
		{
			mov eax, tmp
			jmp eax
		}
		break;
	default:
		retVal = 0;
		break;
	}

	if(argArray)
		delete argArray;

	return retVal;
}


//	Private methods
///////////////////////////
bool CHook::BuildStub()
{
	u8 *codeptr = (u8 *)m_hookFrom;
	u8 *stubptr = (u8 *)entryStub;
	bool retval = true;

	//	Set the memory page of the entry stub to executable, so we can execute it.
	u32 oldprot;
	VirtualProtect(entryStub, sizeof(entryStub), PAGE_EXECUTE_READWRITE, (DWORD *)&oldprot);


	u32 len = 0;
	while(len < 5)
	{
		u32 instrLen = len = mlde32(codeptr);
		memcpy(stubptr, codeptr, len);

		if(IsRelativeBranch(stubptr))
			instrLen = FixRelativeBranch(stubptr, codeptr);

		stubLen += instrLen;

		if(IsRet(stubptr))
		{
			retval = false;
			break;
		}
		
		stubptr += instrLen;
		codeptr += len;
	}
	
	u8 relJmp[] = {INSTR_RELJMP, 0, 0, 0, 0};
	//	Calculate the offset from the end of the stub thus far
	//	to the first instruction after our jump patch.
	u32 offset = MakeDelta(u32, m_hookFrom, entryStub) - 5;

	//	Build the relative jump
	*(u32 *)(relJmp + 1) = offset;
	memcpy(&entryStub[stubLen], relJmp, sizeof(relJmp));

	return retval;
}

void CHook::WriteJump(void *from, void *to)
{
	u32 oldprot;
	u8 relJmp[] = {INSTR_RELJMP, 0, 0, 0, 0};

	VirtualProtect(from, sizeof(relJmp), PAGE_EXECUTE_READWRITE, (DWORD *)&oldprot);
	//	Build the relative jump that will be patched onto from
	u32 offset = MakeDelta(u32, to, from) - 5;
	//	Build the relative jump
	*(u32 *)(relJmp + 1) = offset;
	memcpy(from, relJmp, sizeof(relJmp));
	//	If this one fails, it won't effect the operation of the hook.
	//	So, I don't think it's necessary for Hook() to fail if this
	//	VirtualProtect() does.
	u32 oldprot2;
	VirtualProtect(from, sizeof(relJmp), oldprot, (DWORD *)&oldprot2);
	//	Flush the CPU's instruction cache.  This should always be done
	//	when writing self-modifying code, because the CPU will cache
	//	instructions, and may not detect our newly modified code.
	FlushInstructionCache(GetCurrentProcess(), NULL, 0);
}

void CHook::WriteInt3(void *at)
{
	u32 oldprot;
	VirtualProtect(at, 1, PAGE_EXECUTE_READWRITE, (DWORD *)&oldprot);
	*(u8 *)at = INSTR_INT3;
	u32 oldprot2;
	VirtualProtect(at, 1, oldprot, (DWORD *)&oldprot2);
	FlushInstructionCache(GetCurrentProcess(), NULL, 0);
}

bool CHook::IsRelativeBranch(u8 *instr)
{
	switch(*instr)
	{
	case INSTR_LOOPNZ:
	case INSTR_LOOPE:
	case INSTR_LOOP:
	case INSTR_RELJCX:
	case INSTR_RELCALL:
	case INSTR_RELJMP:
	case INSTR_SHORTJMP:
		return true;
	case INSTR_NEAR_PREFIX:
		if(*(instr + 1) >= INSTR_NEARJCC_BEGIN && *(instr + 1) <= INSTR_NEARJCC_END)
			return true;
	}

	//	Check to see if it's in the valid range of JCC values.
	//	e.g. ja, je, jne, jb, etc..
	if(*instr >= INSTR_SHORTJCC_BEGIN && *instr <= INSTR_SHORTJCC_END)
		return true;

    
	return false;
}

u32 CHook::FixRelativeBranch(u8 *curloc, u8 *prevloc)
{
	u8 *instr = curloc;

	//	This code will determine what type of branch it is, and
	//	increment the pointer to point to the relative branch's
	//	offset.  This has to be modified in the same way for every
	//	instruction.
	switch(*instr)
	{
	case INSTR_SHORTJMP:
		*instr = INSTR_RELJMP;
	case INSTR_RELJCX:
	case INSTR_RELCALL:
	case INSTR_RELJMP:
		instr++;
		break;
	case INSTR_NEAR_PREFIX:
		if(*(instr + 1) >= INSTR_NEARJCC_BEGIN && *(instr + 1) <= INSTR_NEARJCC_END)
			instr += 2;
		break;
	default:
		//	Check to see if it's in the valid range of JCC values.
		//	e.g. ja, je, jne, jb, etc..
		if(*instr >= INSTR_SHORTJCC_BEGIN && *instr <= INSTR_SHORTJCC_END)
		{
			*instr = INSTR_NEAR_PREFIX;
			*(++instr) = INSTR_NEARJCC_BEGIN + (*curloc - INSTR_SHORTJCC_BEGIN);
			instr++;
			break;
		}
		else
			return 0;
	}

	u32 len = MakeDelta(u32, instr, curloc) + sizeof(u32);
	u32 offset = MakeDelta(u32, curloc, prevloc) - len;
	
	return len;
}

bool CHook::IsRet(u8 *instr)
{
	switch(*instr)
	{
		case INSTR_RET:
		case INSTR_RETN:
		case INSTR_RETFN:
		case INSTR_RETF:
			return true;
	}

	return false;
}

int CHook::SetDebugBreak(FARPROC address)
{
	u32 retval;
	HANDLE thSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, NULL);
	THREADENTRY32 te;
	te.dwSize = sizeof(THREADENTRY32);

	Thread32First(thSnap, &te);
	do
	{
		if(te.th32OwnerProcessID != GetCurrentProcessId())
			continue;

		HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, te.th32ThreadID);
		CONTEXT ctx;
		ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
		GetThreadContext(hThread, &ctx);
	
		if(!ctx.Dr0)
		{
			ctx.Dr0 = MakePtr(u32, address, 0);
			ctx.Dr7 |= 0x00000001;
			retval = 0;
		}
		else if(!ctx.Dr1)
		{
			ctx.Dr1 = MakePtr(u32, address, 0);
			ctx.Dr7 |= 0x00000002;
			retval = 1;
		}
		else if(!ctx.Dr2)
		{
			ctx.Dr2 = MakePtr(u32, address, 0);
			ctx.Dr7 |= 0x00000004;
			retval = 2;
		}
		else if(!ctx.Dr3)
		{
			ctx.Dr3 = MakePtr(u32, address, 0);
			ctx.Dr7 |= 0x00000008;
			retval = 3;
		}
		else
			retval = -1;
	
		ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
		SetThreadContext(hThread, &ctx);
		CloseHandle(hThread);
	} while(Thread32Next(thSnap, &te));

	return retval;
}

bool CHook::ClearDebugBreak(u32 index)
{
	HANDLE thSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, NULL);
	THREADENTRY32 te;
	te.dwSize = sizeof(THREADENTRY32);

	Thread32First(thSnap, &te);

	do
	{
		if(te.th32OwnerProcessID != GetCurrentProcessId())
			continue;

		HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, te.th32ThreadID);
		CONTEXT ctx;
		ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
		GetThreadContext(hThread, &ctx);
		switch(index)
		{
		case 0:
			ctx.Dr0 = 0;
			ctx.Dr7 &= 0xFFF0FFFE;	//	Clear the 16-19th and 1st bits
			break;
		case 1:
			ctx.Dr1 = 0;
			ctx.Dr7 &= 0xFF0FFFFD;	//	Clear the 20-23rd and 2nd bits
			break;
		case 2:
			ctx.Dr2 = 0;
			ctx.Dr7 &= 0xF0FFFFFB;	//	Clear the 24-27th and 3rd bits
			break;
		case 3:
			ctx.Dr3 = 0;
			ctx.Dr7 &= 0x0FFFFFF7;	//	Clear the 28-31st and 4th bits
			break;
		default:
			return false;
		}
		SetThreadContext(hThread, &ctx);
		CloseHandle(hThread);
	} while(Thread32Next(thSnap, &te));

	return true;
}

LONG CALLBACK CHook::DebugHookHandler(PEXCEPTION_POINTERS ExceptionInfo)
{

	if(ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_SINGLE_STEP)
	{
		u32 i;
		for(i = 0; i < 4; i++)
			if(ExceptionInfo->ContextRecord->Eip == MakePtr(u32, debugHooks.targetFuncs[i], 0))
				break;

		//	Make sure this single step exception was actually one of ours
		if(i >= 4)
			return EXCEPTION_CONTINUE_SEARCH;

		//	Set the instruction pointer to the address of our hook
		ExceptionInfo->ContextRecord->Eip = MakePtr(u32, debugHooks.hookFuncs[i], 0);
		return EXCEPTION_CONTINUE_EXECUTION;
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

LONG CALLBACK CHook::Int3HookHandler(PEXCEPTION_POINTERS ExceptionInfo)
{

	if(ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_BREAKPOINT)
	{
		if(!int3hooks[MakePtr(FARPROC, ExceptionInfo->ContextRecord->Eip, 0)])
			return EXCEPTION_CONTINUE_SEARCH;

		//	Set the instruction pointer to the address of our hook
		ExceptionInfo->ContextRecord->Eip = MakePtr(u32, int3hooks[MakePtr(FARPROC, ExceptionInfo->ContextRecord->Eip, 0)], 0);
		return EXCEPTION_CONTINUE_EXECUTION;
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

//	Called every time a new thread is created, from within the
//	new thread.  Also, it's called before any of the thread's
//	code executes.
u32 CHook::ThreadCallback()
{
	if(debugHooks.targetFuncs[0])
		SetDebugBreak(debugHooks.targetFuncs[0]);
	if(debugHooks.targetFuncs[1])
		SetDebugBreak(debugHooks.targetFuncs[1]);
	if(debugHooks.targetFuncs[2])
		SetDebugBreak(debugHooks.targetFuncs[2]);
	if(debugHooks.targetFuncs[3])
		SetDebugBreak(debugHooks.targetFuncs[3]);

	return threadHook->OriginalFunc(0);
}

//	Matt Pietrek's function
IMAGE_SECTION_HEADER *CHook::GetEnclosingSectionHeader(u32 rva) 
{
	IMAGE_SECTION_HEADER *section = IMAGE_FIRST_SECTION(ntHd); 
	for (u32 i = 0; i < ntHd->FileHeader.NumberOfSections; i++, section++ )
	{
		// This 3 line idiocy is because Watcom's linker actually sets the
		// Misc.VirtualSize field to 0.  (!!! - Retards....!!!)
		u32 size = section->Misc.VirtualSize;
		if ( 0 == size )
			size = section->SizeOfRawData;

		// Is the RVA within this section?
		if ( (rva >= section->VirtualAddress) && 
			(rva < (section->VirtualAddress + size)))
			return section;
	}

	return NULL;
}

unsigned long CHook::GetMappedSectionOffset(IMAGE_SECTION_HEADER *seHd)
{
	IMAGE_SECTION_HEADER *section = IMAGE_FIRST_SECTION(ntHd);
	u32 offset = MakeDelta(u32, section, dosHd);
	for(u32 i = 0; i < ntHd->FileHeader.NumberOfSections; i++, section++)
	{
		if(section->Name == seHd->Name)
		{
			offset = MakeDelta(u32, section->VirtualAddress, section->PointerToRawData);
			break;
		}
	}

	return offset;
}

//	This function is also Pietrek's, with a modification by me so that it can handle
//	images that are mapped into memory.
void *CHook::GetPtrFromRVA(u32 rva, bool mapped)
{
	IMAGE_SECTION_HEADER *pSectionHdr = GetEnclosingSectionHeader(rva);
	s32 offset = 0;

	if(mapped)
		offset = GetMappedSectionOffset(pSectionHdr);
	if (!pSectionHdr)
		return 0;

	s32 delta = (s32)(pSectionHdr->VirtualAddress-pSectionHdr->PointerToRawData);
	return (void *) ( (u8 *)dosHd + rva - delta + offset);
}