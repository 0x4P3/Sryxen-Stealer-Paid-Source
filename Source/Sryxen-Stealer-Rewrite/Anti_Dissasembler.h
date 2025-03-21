#pragma once
#include "Windows.h"

VOID AntiDisassmConstantCondition();
VOID AntiDisassmAsmJmpSameTarget();
VOID AntiDisassmImpossibleDiasassm();
VOID AntiDisassmFunctionPointer();
VOID AntiDisassmReturnPointerAbuse();

VOID RunAllAntiDisassmTechniques();

