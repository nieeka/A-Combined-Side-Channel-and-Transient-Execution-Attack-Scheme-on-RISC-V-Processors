// See LICENSE for license details

#include <stdint.h>
#include "encoding.h"
#include "vm.h"

extern uint8_t secret[PGSIZE];

void pmp_boot() {
    uintptr_t pmpc = PMP_NAPOT | PMP_R | PMP_W | PMP_X;
    uintptr_t pmpa = ((uintptr_t)1 << (__riscv_xlen == 32 ? 31 : 53)) - 1;
    asm volatile("la t0, 1f\n\t"
                 "csrrw t0, mtvec, t0\n\t"
                 "csrw pmpaddr7, %1\n\t"
                 "addi a0, %0, 0\n\t"
                 "slli a0, a0, 56\n\t"
                 "csrw pmpcfg0, a0\n\t"
                 ".align 2\n\t"
                 "1: csrw mtvec, t0"
                 : : "r" (pmpc), "r" (pmpa) : "t0", "a0");

    pmpc = PMP_NAPOT;
    pmpa = ((uintptr_t) secret | (((uintptr_t)1 << 11) - 1)) >> 2;
    asm volatile("csrw pmpaddr0, %1\n\t"
                 "csrs pmpcfg0, %0\n\t"
                 : : "r" (pmpc), "r" (pmpa));

    return;
}
