//
// Created by mainuser on 05.03.16.
//

#ifndef RUC2_MIPS_DEFS_H
#define RUC2_MIPS_DEFS_H

typedef enum
{
    $0, $at,
    $v0, $v1,
    $a0, $a1, $a2, $a3,
    $t0, $t1, $t2, $t3, $t4, $t5, $t6, $t7,
    $s0, $s1, $s2, $s3, $s4, $s5, $s6, $s7,
    $t8, $t9,
    $k0, $k1,
    $gp, $sp, $fp, $ra
} Registers;

typedef enum
{
    LABEL,
    J,
    JR,
    JAL,
    LI_,
    LA_,
    LW,
    SW,
    ADD,
    ADDU,
    ADDI,
    ADDIU,
    MOVE,
    SYSCALL
} Instructions;

typedef enum
{
    SAVED,
    TEMP,
    STACK,
    MEM,
    STATIC,
    GARBAGE,
    OTHER
} Emplacement;

#endif //RUC2_MIPS_DEFS_H
