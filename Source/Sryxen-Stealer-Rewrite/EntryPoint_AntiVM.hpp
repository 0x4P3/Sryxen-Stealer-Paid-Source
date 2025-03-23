#ifndef ENTRYPOINT_ANTIVM_HPP
#define ENTRYPOINT_ANTIVM_HPP

#include <Windows.h>
#include "Anti_Triage.h"
#include <iostream>

inline void RunAllAntiVM() {
    if (IsProcessRunning(OBF(L"sysmon.exe"))) {
        OutputDebugStringW(OBF(L"really detected?."));
        exit(1);
    }

}

#endif
