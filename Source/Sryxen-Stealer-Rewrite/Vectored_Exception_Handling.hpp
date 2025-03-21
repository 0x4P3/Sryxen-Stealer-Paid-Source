#ifndef VECTORED_EXCEPTION_HANDLING_HPP
#define VECTORED_EXCEPTION_HANDLING_HPP

#include <windows.h>
#include "winapi_structs.hpp"
// declaring so we can construct the STUB
DWORD t = 0;
LPVOID m_indx = (LPVOID)HiddenCalls::CustomGetProcAddress(HiddenCalls::CustomGetModuleHandleW(L"Ntdll.dll"), "NtDrawText");
//

// declaring RIP from veh-stub.asm
extern "C" VOID RIP();

// the stub itself
LONG WINAPI VectExceptionHandler(PEXCEPTION_POINTERS pExceptionInfo) {
    if (pExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION) {
        pExceptionInfo->ContextRecord->R10 = pExceptionInfo->ContextRecord->Rcx;  
        RIP();
        pExceptionInfo->ContextRecord->Rax = t;  
        RIP();
        pExceptionInfo->ContextRecord->Rip = (DWORD64)((DWORD64)m_indx + 0x12);
        RIP();
        return EXCEPTION_CONTINUE_EXECUTION; 
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

#endif
