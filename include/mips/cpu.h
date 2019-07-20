#pragma once

typedef struct
{
    uint64_t value;

    void (*write_callback)(void);
    void (*read_callback)(void);
} reg_t;

typedef struct
{
    reg_t GPR[32];
    reg_t FPR[32]; // 0: FCR0 (32-bit), 31: FCR31 (32-bit)
    reg_t COP0[32];
    reg_t PC;
    reg_t HI;
    reg_t LO;
    bool  LLbit;
    bool  COC1;
} regs_t;

typedef struct
{
    regs_t regs;
    bool is_branching;

    uint32_t curr_target;

    uint8_t  curr_inst_cycles;
    uint64_t cycles;
    uint64_t all_cycles;
} cpu_t;