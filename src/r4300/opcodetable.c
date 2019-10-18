#include "r4300/opcodetable.h"

#include "common.h"

void CPU_opcode_table_init(void)
{
    memset(CPU_opcode_table, 0, sizeof(CPU_opcode_table));
    CPU_opcode_table[0b000000].interpret = &SPECIAL;
    CPU_opcode_table[0b000001].interpret = &REGIMM;
    CPU_opcode_table[0b010000].interpret = &COP0;
    CPU_opcode_table[0b010001].interpret = &COP1;
    CPU_opcode_table[0b110101].interpret = &LDC1;
    CPU_opcode_table[0b110001].interpret = &LWC1;
    CPU_opcode_table[0b111101].interpret = &SDC1;
    CPU_opcode_table[0b111001].interpret = &SWC1;
    
    CPU_opcode_table[0b001000].interpret = &ADDI;
    CPU_opcode_table[0b001001].interpret = &ADDIU;
	CPU_opcode_table[0b011000].interpret = &DADDI;
	CPU_opcode_table[0b011001].interpret = &DADDIU;
    CPU_opcode_table[0b001100].interpret = &ANDI;
	CPU_opcode_table[0b001101].interpret = &ORI;
	CPU_opcode_table[0b001110].interpret = &XORI;

    CPU_opcode_table[0b100000].interpret = &LB;
    CPU_opcode_table[0b100100].interpret = &LBU;
    CPU_opcode_table[0b110111].interpret = &LD;
    CPU_opcode_table[0b011010].interpret = &LDL;
    CPU_opcode_table[0b011011].interpret = &LDR;
    CPU_opcode_table[0b100001].interpret = &LH;
    CPU_opcode_table[0b100101].interpret = &LHU;
    CPU_opcode_table[0b110000].interpret = &LL;
    CPU_opcode_table[0b110100].interpret = &LLD;
    CPU_opcode_table[0b001111].interpret = &LUI;
    CPU_opcode_table[0b100011].interpret = &LW;
    CPU_opcode_table[0b100010].interpret = &LWL;
    CPU_opcode_table[0b100110].interpret = &LWR;
    CPU_opcode_table[0b100111].interpret = &LWU;

    CPU_opcode_table[0b101000].interpret = &SB;
    CPU_opcode_table[0b111000].interpret = &SC;
    CPU_opcode_table[0b111100].interpret = &SCD;
    CPU_opcode_table[0b111111].interpret = &SD;
    CPU_opcode_table[0b101100].interpret = &SDL;
    CPU_opcode_table[0b101101].interpret = &SDR;
    CPU_opcode_table[0b101001].interpret = &SH;
    CPU_opcode_table[0b001010].interpret = &SLTI;
    CPU_opcode_table[0b001011].interpret = &SLTIU;
    CPU_opcode_table[0b101011].interpret = &SW;
    CPU_opcode_table[0b101010].interpret = &SWL;
    CPU_opcode_table[0b101110].interpret = &SWR;

    CPU_opcode_table[0b000100].interpret = &BEQ;
    CPU_opcode_table[0b010100].interpret = &BEQL;
    CPU_opcode_table[0b000111].interpret = &BGTZ;
    CPU_opcode_table[0b010111].interpret = &BGTZL;
    CPU_opcode_table[0b000110].interpret = &BLEZ;
    CPU_opcode_table[0b010110].interpret = &BLEZL;
    CPU_opcode_table[0b000101].interpret = &BNE;
    CPU_opcode_table[0b010101].interpret = &BNEL;

    CPU_opcode_table[0b000010].interpret = &J;
    CPU_opcode_table[0b000011].interpret = &JAL;

    CPU_opcode_table[0b101111].interpret = &CACHE;
}