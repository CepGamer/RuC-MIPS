//  RuC2
//
//  Created by Sergei Bolotov on 05/Mar/2016.
//  Copyright (c) 2016 Sergei Bolotov. All rights reserved.
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
    DELETED,
    /* переходы */
    J,
    JR,
    JAL,
    BEQ,
    BEQI,
    BEQZ,
    LI_,
    LA_,
    LW,
    SW,
    /* унарные операции */
    NEGU,
    /* бинарные операции */
    /* арифметические */
    ADD,
    ADDU,
    SUB,
    SUBU,
    ADDI,
    ADDIU,

    /* умножение и деление */
    MUL,
    DIV,
    REM,

    /* логические */
    OR,
    XOR,
    AND,
    SLL,
    SLLV,
    SRA,
    SRAV,
    /* сравнение */
    SLT,
    SGT,
    SLE,
    SGE,
    SEQ,
    SEQI,
    /* прочие */
    MOVE,
    SYSCALL
} Instructions;

typedef enum
{
    SAVED,      /* сохранённый регистр */
    TEMP,       /* временный регистр */
    STACK,      /* стэк */
    MEM,        /* память */
    STATIC,     /* значение известно на этапе компиляции */
    GARBAGE,    /* мусор */
    OTHER,      /* прочее */
    IDENT_      /* идентификатор */
} Emplacement;

typedef enum
{
    UPDATESINLOOP   = 0x01,
    CONSTANT        = 0x02,

    ALL             = 0xffff
} Flags;

typedef union {
    int integer;
    float floating;
    char character;
    void * pointer;
} Value;

typedef struct
{
    char * name;
    int type;
    int size;
    Value value;
} IdentEntry;

typedef struct
{
    Emplacement emplacement;
    Value value;
    int flags;
} ValueEntry;

typedef struct
{
    Instructions code;
    int first_op;
    int second_op;
    int third_op;
} Instruction;

typedef struct
{
    int code;
    Value value;
} Operation;

#endif //RUC2_MIPS_DEFS_H
