#include "opcodetable.h"
#include "interpreter.h"

void OpcodeTableInit(void)
{
    OpcodeTable[0b000000].Interpret = &SPECIAL;
    OpcodeTable[0b000001].Interpret = &REGIMM;
    OpcodeTable[0b010000].Interpret = &COPz;
    
    OpcodeTable[0b001000].Interpret = &ADDI;
    OpcodeTable[0b001001].Interpret = &ADDIU;
	OpcodeTable[0b011000].Interpret = &DADDI;
	OpcodeTable[0b011001].Interpret = &DADDIU;
    OpcodeTable[0b001100].Interpret = &ANDI;
	OpcodeTable[0b001101].Interpret = &ORI;
	OpcodeTable[0b001110].Interpret = &XORI;

    OpcodeTable[0b100000].Interpret = &LB;
    OpcodeTable[0b100100].Interpret = &LBU;
    OpcodeTable[0b110111].Interpret = &LD;
    OpcodeTable[0b011010].Interpret = &LDL;
    OpcodeTable[0b011011].Interpret = &LDR;
    OpcodeTable[0b100001].Interpret = &LH;
    OpcodeTable[0b100101].Interpret = &LHU;
    OpcodeTable[0b110000].Interpret = &LL;
    OpcodeTable[0b110100].Interpret = &LLD;
    OpcodeTable[0b001111].Interpret = &LUI;
    OpcodeTable[0b100011].Interpret = &LW;
    OpcodeTable[0b100010].Interpret = &LWL;
    OpcodeTable[0b100110].Interpret = &LWR;
    OpcodeTable[0b100111].Interpret = &LWU;

    OpcodeTable[0b101000].Interpret = &SB;
    OpcodeTable[0b111000].Interpret = &SC;
    OpcodeTable[0b111100].Interpret = &SCD;
    OpcodeTable[0b111111].Interpret = &SD;
    OpcodeTable[0b101100].Interpret = &SDL;
    OpcodeTable[0b101101].Interpret = &SDR;
    OpcodeTable[0b101001].Interpret = &SH;
    OpcodeTable[0b001010].Interpret = &SLTI;
    OpcodeTable[0b001011].Interpret = &SLTIU;
    OpcodeTable[0b101011].Interpret = &SW;
    OpcodeTable[0b101010].Interpret = &SWL;
    OpcodeTable[0b101110].Interpret = &SWR;

    OpcodeTable[0b000100].Interpret = &BEQ;
    OpcodeTable[0b010100].Interpret = &BEQL;
    OpcodeTable[0b000111].Interpret = &BGTZ;
    OpcodeTable[0b010111].Interpret = &BGTZL;
    OpcodeTable[0b000110].Interpret = &BLEZ;
    OpcodeTable[0b010110].Interpret = &BLEZL;
    OpcodeTable[0b000101].Interpret = &BNE;
    OpcodeTable[0b010101].Interpret = &BNEL;

    OpcodeTable[0b000010].Interpret = &J;
    OpcodeTable[0b000011].Interpret = &JAL;

    OpcodeTable[0b101111].Interpret = &CACHE;
}