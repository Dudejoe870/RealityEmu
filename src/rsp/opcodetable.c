#include "rsp/opcodetable.h"

#include "common.h"

void RSP_opcode_table_init(void)
{
    RSP_opcode_table[0b000000].interpret = &SPECIAL;
    RSP_opcode_table[0b000001].interpret = &REGIMM;
    RSP_opcode_table[0b010000].interpret = &COP0;
    
    RSP_opcode_table[0b001000].interpret = &ADDI;
    RSP_opcode_table[0b001001].interpret = &ADDIU;
    RSP_opcode_table[0b001100].interpret = &ANDI;
	RSP_opcode_table[0b001101].interpret = &ORI;
	RSP_opcode_table[0b001110].interpret = &XORI;

    RSP_opcode_table[0b100000].interpret = &LB;
    RSP_opcode_table[0b100100].interpret = &LBU;
    RSP_opcode_table[0b100001].interpret = &LH;
    RSP_opcode_table[0b100101].interpret = &LHU;
    RSP_opcode_table[0b001111].interpret = &LUI;
    RSP_opcode_table[0b100011].interpret = &LW;

    RSP_opcode_table[0b101000].interpret = &SB;
    RSP_opcode_table[0b101001].interpret = &SH;
    RSP_opcode_table[0b001010].interpret = &SLTI;
    RSP_opcode_table[0b001011].interpret = &SLTIU;
    RSP_opcode_table[0b101011].interpret = &SW;

    RSP_opcode_table[0b000100].interpret = &BEQ;
    RSP_opcode_table[0b000111].interpret = &BGTZ;
    RSP_opcode_table[0b000110].interpret = &BLEZ;
    RSP_opcode_table[0b000101].interpret = &BNE;

    RSP_opcode_table[0b000010].interpret = &J;
    RSP_opcode_table[0b000011].interpret = &JAL;

    RSP_opcode_table[0b101111].interpret = &CACHE;
}