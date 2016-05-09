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
    $gp, $sp, $fp, $ra,
    /* регистры с плавающей запятой */
    $f0, $f1, $f2, $f3, $f4, $f5, $f6, $f7,
    $f8, $f9, $f10, $f11, $f12, $f13, $f14, $f15,
    $f16, $f17, $f18, $f19, $f20, $f21, $f22, $f23,
    $f24, $f25, $f26, $f27, $f28, $f29, $f30, $f31

} Registers;

typedef enum
{
    LABEL = 1,
    DELETED,
    /* переходы */
    J,
    JR,
    JAL,
    JALR,
    BEQ,
    BEQI,
    BEQZ,
    BNEZ,
    LI_,
    LA_,
    LW,
    SW,
    SWC1,
    /* унарные операции */
    NEGU,
    /* бинарные операции */
    /* арифметические */
    ADD,
    ADDU,
    SUB,
    SUBU,
    SUBIU,
    ADDI,
    ADDIU,

    ADDS,
    SUBS,

    /* умножение и деление */
    MUL,
    MULI,
    DIV,
    DIVI,
    REM,
    REMI,

    MULS,
    DIVS,

    /* логические */
    OR,
    ORI,
    XOR,
    XORI,
    AND,
    ANDI,
    SLL,
    SLLV,
    SRA,
    SRAV,
    /* сравнение */
    SLT,
    SLTI,
    SGT,
    SGTI,
    SLE,
    SLEI,
    SGE,
    SGEI,
    SEQ,
    SEQI,
    SNE,
    SNEI,

    /** сравнение с плавающей запятой */
    CEQS,
    CLES,
    CLTS,
    /* прочие */
    MOVE,
    MOVT,
    MOVF,
    SYSCALL
} Instructions;

typedef enum
{
    SAVED,      /* сохранённый регистр */
    TEMP,       /* временный регистр */
    ARG,        /* регистр аргументов */
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
    Instructions code;
    int first_op;
    int second_op;
    int third_op;
} Instruction;

typedef struct
{
    char * name;
    int type;
    int size;
    Value value;
    int label;
} IdentEntry;

typedef struct
{
    Emplacement emplacement;
    Value value;
    int flags;
    Instruction *previous_save;
    int identref;
} ValueEntry;

typedef struct
{
    ValueEntry old;
    int ident;
} IdentDiff;

typedef struct
{
    ValueEntry old;
    ValueEntry *ident;
} ValueDiff;

typedef struct
{
    int code;
    Value value;
} Operation;

#endif //RUC2_MIPS_DEFS_H
