#include "opcodetable.h"
#include "interpreter.h"

void OpcodeTableInit(void)
{
    OpcodeTable[0b000000].Interpret = &SPECIAL;
    OpcodeTable[0b000001].Interpret = &REGIMM;
    
    OpcodeTable[0b001000].Interpret = &ADDI;
    OpcodeTable[0b001001].Interpret = &ADDIU;
	OpcodeTable[0b011000].Interpret = &DADDI;
	OpcodeTable[0b011001].Interpret = &DADDIU;
    OpcodeTable[0b001100].Interpret = &ANDI;
	OpcodeTable[0b001101].Interpret = &ORI;
	OpcodeTable[0b001110].Interpret = &XORI;

    

    OpcodeTable[0b000100].Interpret = &BEQ;
    OpcodeTable[0b010100].Interpret = &BEQL;
}