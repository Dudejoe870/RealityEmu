#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct
{
    uint64_t value;

    void (*write_callback)(uint64_t value, uint8_t index);
    void (*read_callback)(uint8_t index);
} reg_t;

typedef struct
{
    reg_t GPR[32];
    reg_t FPR[32]; // 0: FCR0 (32-bit), 31: FCR31 (32-bit), R4300 only
    reg_t COP0[32]; // R4300: 32, 
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

    bool rsp;
} cpu_t;

typedef struct
{
    void (*interpret)(uint32_t value, cpu_t* cpu);
} opcode_t;

bool is_running;

__attribute__((__always_inline__)) static inline void write_GPR(uint64_t value, uint8_t index, cpu_t* cpu)
{
    cpu->regs.GPR[index].value = value;
    if (cpu->regs.GPR[index].write_callback) cpu->regs.GPR[index].write_callback(value, index);
}

__attribute__((__always_inline__)) static inline uint64_t read_GPR(uint8_t index, cpu_t* cpu)
{
    if (cpu->regs.GPR[index].read_callback) cpu->regs.GPR[index].read_callback(index);
    return cpu->regs.GPR[index].value;
}

__attribute__((__always_inline__)) static inline void write_FPR(uint64_t value, uint8_t index, cpu_t* cpu) // R4300 only
{
    bool FR = (cpu->regs.COP0[12].value & 0x04000000) > 0; // COP0_STATUS

    if (index & 1 && !FR) return;
    
    cpu->regs.FPR[index].value = value;
    if (cpu->regs.FPR[index].write_callback) cpu->regs.FPR[index].write_callback(value, index);
}

__attribute__((__always_inline__)) static inline uint64_t read_FPR(uint8_t index, cpu_t* cpu) // R4300 only
{
    bool FR = (cpu->regs.COP0[12].value & 0x04000000) > 0; // COP0_STATUS

    if (index & 1 && !FR) return 0;

    if (cpu->regs.FPR[index].read_callback) cpu->regs.FPR[index].read_callback(index);
    return cpu->regs.FPR[index].value;
}

__attribute__((__always_inline__)) static inline void write_COP0(uint64_t value, uint8_t index, cpu_t* cpu)
{
    if (cpu->rsp && index > 15) return;
    cpu->regs.COP0[index].value = value;
    if (cpu->regs.COP0[index].write_callback) cpu->regs.COP0[index].write_callback(value, index);
}

__attribute__((__always_inline__)) static inline uint64_t read_COP0(uint8_t index, cpu_t* cpu)
{
    if (cpu->rsp && index > 15) return 0;
    if (cpu->regs.COP0[index].read_callback) cpu->regs.COP0[index].read_callback(index);
    return cpu->regs.COP0[index].value;
}

__attribute__((__always_inline__)) static inline void write_PC(uint32_t value, cpu_t* cpu)
{
    cpu->regs.PC.value = (uint64_t)value;
    if (cpu->regs.PC.write_callback) cpu->regs.PC.write_callback(value, 0);
}

__attribute__((__always_inline__)) static inline uint32_t read_PC(cpu_t* cpu)
{
    if (cpu->regs.PC.read_callback) cpu->regs.PC.read_callback(0);
    return cpu->regs.PC.value;
}

__attribute__((__always_inline__)) static inline void advance_PC(cpu_t* cpu)
{
    write_PC(read_PC(cpu) + 4, cpu);
}

__attribute__((__always_inline__)) static inline void write_HI(uint64_t value, cpu_t* cpu)
{
    cpu->regs.HI.value = value;
    if (cpu->regs.HI.write_callback) cpu->regs.HI.write_callback(value, 0);
}

__attribute__((__always_inline__)) static inline uint64_t read_HI(cpu_t* cpu)
{
    if (cpu->regs.HI.read_callback) cpu->regs.HI.read_callback(0);
    return cpu->regs.HI.value;
}

__attribute__((__always_inline__)) static inline void write_LO(uint64_t value, cpu_t* cpu)
{
    cpu->regs.LO.value = value;
    if (cpu->regs.LO.write_callback) cpu->regs.LO.write_callback(value, 0);
}

__attribute__((__always_inline__)) static inline uint64_t read_LO(cpu_t* cpu)
{
    if (cpu->regs.LO.read_callback) cpu->regs.LO.read_callback(0);
    return cpu->regs.LO.value;
}