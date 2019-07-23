#include "r4300/debug.h"

#include "common.h"

char* gpr_names[] =
{
    "r0",
    "at",
    "v0",
    "v1",
    "a0",
    "a1",
    "a2",
    "a3",
    "t0",
    "t1",
    "t2",
    "t3",
    "t4",
    "t5",
    "t6",
    "t7",
    "s0",
    "s1",
    "s2",
    "s3",
    "s4",
    "s5",
    "s6",
    "s7",
    "t8",
    "t9",
    "k0",
    "k1",
    "gp",
    "sp",
    "s8",
    "ra"
};

char* cpu_cop0_names[] =
{
    "Index",
    "Random",
    "EntryLo0",
    "EntryLo1",
    "Context",
    "PageMask",
    "Wired",
    "Reserved",
    "BadVAddr",
    "Count",
    "EntryHi",
    "Compare",
    "Status",
    "Cause",
    "EPC",
    "PRid",
    "Config",
    "LLAddr",
    "WatchLo",
    "WatchHi",
    "XContext",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "CacheErr",
    "TagLo",
    "TagHi",
    "ErrorEPC",
    "Reserved"
};

char* rsp_cop0_names[] =
{
    "DMA Cache",
    "DMA DRAM",
    "DMA Read Length",
    "DMA Write Length",
    "SP Status",
    "DMA Full",
    "DMA Busy",
    "SP Reserved",
    "CMD Start",
    "CMD End",
    "CMD Current",
    "CMD Status",
    "CMD Clock",
    "CMD Busy",
    "CMD Pipe Busy",
    "CMD TMEM Busy"
};

void dbg_memdump(uint32_t start, uint32_t end)
{
    FILE* fptr;

    if ((fptr = fopen("memdump.bin", "wb")) == NULL)
    {
        printf("Can't create memdump: Can't write to file!");

        exit(1);
    }

    for (uint32_t addr = start; addr <= end; ++addr)
    {
        uint8_t data = read_uint8(addr);
        fwrite(&data, sizeof(uint8_t), 1, fptr);
    }
    fclose(fptr);

    exit(0);
}

void dbg_printreg(cpu_t cpu)
{
    printf("PC = 0x%x\n", (uint32_t)cpu.regs.PC.value);
    for (size_t i = 0; i < 32; ++i)
        printf("%s = 0x%lx\n", gpr_names[i], cpu.regs.GPR[i].value);
    if (!cpu.rsp)
    {
        printf("HI = 0x%lx\n", cpu.regs.HI.value);
        printf("LO = 0x%lx\n", cpu.regs.LO.value);
        printf("LLbit = %u\n", cpu.regs.LLbit);
    }
    putchar('\n');
    char** cop0_names = (cpu.rsp) ? rsp_cop0_names : cpu_cop0_names;
    for (size_t i = 0; i < ((cpu.rsp) ? 16 : 32); ++i)
        printf("%s = 0x%lx\n", cop0_names[i], cpu.regs.COP0[i].value);
    if (cpu.rsp)
    {
        for (size_t i = 16; i < 32; ++i)
            printf("Undefined = 0x%lx\n", cpu.regs.COP0[i].value);
    }
    if (!cpu.rsp)
    {
        putchar('\n');
        for (size_t i = 0; i < 32; ++i)
            printf("FPR[%lu] = 0x%lx\n", i, cpu.regs.FPR[i].value);
    }
}