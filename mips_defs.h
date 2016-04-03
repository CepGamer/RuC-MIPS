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
    //  переходы
    J,
    JR,
    JAL,
    BEQZ,
    LI_,
    LA_,
    LW,
    SW,
    //  бинарные операции
    ADD,
    ADDU,
    ADDI,
    ADDIU,
    //  сравнение
    SLT,
    SGT,
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

typedef enum
{
    UPDATESINLOOP   = 0x01,
    CONSTANT        = 0x02
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
    int flags;
} DataEntry;

typedef struct
{
    Emplacement emplacement;
    Value value;
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
