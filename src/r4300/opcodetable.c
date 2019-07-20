#include "common.h"

void opcode_table_init(void)
{
    opcode_table[0b000000].interpret = &SPECIAL;
    opcode_table[0b000001].interpret = &REGIMM;
    opcode_table[0b010000].interpret = &COP0;
    opcode_table[0b010001].interpret = &COP1;
    opcode_table[0b110101].interpret = &LDC1;
    opcode_table[0b110001].interpret = &LWC1;
    opcode_table[0b111101].interpret = &SDC1;
    opcode_table[0b111001].interpret = &SWC1;
    
    opcode_table[0b001000].interpret = &ADDI;
    opcode_table[0b001001].interpret = &ADDIU;
	opcode_table[0b011000].interpret = &DADDI;
	opcode_table[0b011001].interpret = &DADDIU;
    opcode_table[0b001100].interpret = &ANDI;
	opcode_table[0b001101].interpret = &ORI;
	opcode_table[0b001110].interpret = &XORI;

    opcode_table[0b100000].interpret = &LB;
    opcode_table[0b100100].interpret = &LBU;
    opcode_table[0b110111].interpret = &LD;
    opcode_table[0b011010].interpret = &LDL;
    opcode_table[0b011011].interpret = &LDR;
    opcode_table[0b100001].interpret = &LH;
    opcode_table[0b100101].interpret = &LHU;
    opcode_table[0b110000].interpret = &LL;
    opcode_table[0b110100].interpret = &LLD;
    opcode_table[0b001111].interpret = &LUI;
    opcode_table[0b100011].interpret = &LW;
    opcode_table[0b100010].interpret = &LWL;
    opcode_table[0b100110].interpret = &LWR;
    opcode_table[0b100111].interpret = &LWU;

    opcode_table[0b101000].interpret = &SB;
    opcode_table[0b111000].interpret = &SC;
    opcode_table[0b111100].interpret = &SCD;
    opcode_table[0b111111].interpret = &SD;
    opcode_table[0b101100].interpret = &SDL;
    opcode_table[0b101101].interpret = &SDR;
    opcode_table[0b101001].interpret = &SH;
    opcode_table[0b001010].interpret = &SLTI;
    opcode_table[0b001011].interpret = &SLTIU;
    opcode_table[0b101011].interpret = &SW;
    opcode_table[0b101010].interpret = &SWL;
    opcode_table[0b101110].interpret = &SWR;

    opcode_table[0b000100].interpret = &BEQ;
    opcode_table[0b010100].interpret = &BEQL;
    opcode_table[0b000111].interpret = &BGTZ;
    opcode_table[0b010111].interpret = &BGTZL;
    opcode_table[0b000110].interpret = &BLEZ;
    opcode_table[0b010110].interpret = &BLEZL;
    opcode_table[0b000101].interpret = &BNE;
    opcode_table[0b010101].interpret = &BNEL;

    opcode_table[0b000010].interpret = &J;
    opcode_table[0b000011].interpret = &JAL;

    opcode_table[0b101111].interpret = &CACHE;
}