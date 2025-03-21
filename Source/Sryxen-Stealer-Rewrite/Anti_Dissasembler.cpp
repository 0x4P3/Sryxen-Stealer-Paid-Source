#include "Anti_Dissasembler.h"
#include "Windows.h"
/*
	This technique is composed of a single conditional jump instruction placed where the condition
	will always be the same.
*/
VOID AntiDisassmConstantCondition()
{

}

/*
	The most common anti-disassembly technique seen in the wild is two back-to back
	conditional jump instructions that both point to the same target. For example,
	if a jz XYZ is followed by jnz XYZ, the location XYZ will always be jumped to
*/
VOID AntiDisassmAsmJmpSameTarget()
{

}


/*
	By using a data byte placed strategically after a conditional jump instruction
	with the idea that disassembly starting at this byte will prevent the real instruction
	that follows from being disassembled because the byte that inserted is the opcode for
	a multibyte instruction.

*/
VOID AntiDisassmImpossibleDiasassm()
{

}


/*
	If function pointers are used in handwritten assembly or crafted in a nonstandard way
	in source code, the results can be difficult to reverse engineer without dynamic analysis.
*/
VOID AntiDisassmFunctionPointer()
{

	DWORD Number = 2;

}


/*
	The most obvious result of this technique is that the disassembler doesn t show any
	code cross - reference to the target being jumped to.
*/
VOID AntiDisassmReturnPointerAbuse()
{

}



VOID RunAllAntiDisassmTechniques()
{
	AntiDisassmConstantCondition();
	AntiDisassmAsmJmpSameTarget();
	AntiDisassmImpossibleDiasassm();
	AntiDisassmFunctionPointer();
	AntiDisassmReturnPointerAbuse();
}