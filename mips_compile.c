//  RuC2
//
//  Created by BSergei Bolotov on 03/Mar/2016.
//  Copyright (c) 2016 Sergei Bolotov. All rights reserved.
//

#include "global_vars.h"
#include "mips_defs.h"

char * outfile = "result.asm";
FILE * out_mips;

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

Operation temp_regs[] = {0,0,0,0,0,0,0,0,0,0};
int temp_regs_count = 10;
Operation saved_regs[] = {0,0,0,0,0,0,0,0};
int saved_regs_count = 9;
Operation op_stack[100] = {0};
Operation val_stack[1000] = {0};
ValueEntry mips_identref[10000] = {0};
int op_sp, val_sp;

DataEntry data_entries[1000];
Instruction initialise_instr[1000], code_instr[10000];

int curTree, curData, curInit, curCode;
int level;

char * name_from_identref(int identref)
{
    return reprtab + ((identab[identref + 1] + 2 < 28 ? 8 : identab[identref + 1]) + 2);
}

char * reg_to_string(Registers reg)
{
    switch (reg) {
    case $0:
        return "$0";
    case $v0:
        return "$v0";
    case $v1:
        return "$v1";
    case $t0:
        return "$t0";
    case $sp:
        return "$sp";
    case $ra:
        return "$ra";
    default:
        return "";
    }
}

void write_op(Instruction instr)
{
    switch (instr.code) {
    case J:
        fprintf(output, "j\t%s\nnop\n", (char *)instr.first_op);
        break;
    case JR:
        fprintf(output, "jr\t%s\nnop\n", reg_to_string(instr.first_op));
        break;
    case LABEL:
        fprintf(output, "%s:\n", (char *)instr.first_op);
        break;
    case ADDI:
        fprintf(output, "addi\t%s,%s,%d\n"
                , reg_to_string(instr.first_op)
                , reg_to_string(instr.second_op)
                , instr.third_op);
        break;
    case MOVE:
        fprintf(output, "move\t%s,%s\n"
                , reg_to_string(instr.first_op)
                , reg_to_string(instr.second_op));
        break;
    case LA_:
        fprintf(output, "la\t%s,%s\n"
                , reg_to_string(instr.first_op)
                , (char*)instr.second_op);
        break;
    case LW:
        fprintf(output, "lw\t%s,%d(%s)\n"
                , reg_to_string(instr.first_op)
                , instr.third_op
                , reg_to_string(instr.second_op));
        break;
    default:
        break;
    }
}

void write_data(DataEntry data)
{
    fprintf(output, "%s:\t", data.name);
    switch(data.type)
    {
    case LINT:
        fprintf(output, ".word\t%d\n", data.value.integer);
        break;
    }
}

void process_declaration();
void process_function();
void process_expression();

Registers select_tempreg()
{
    return $t0;
}

Registers get_reg_from_instr(Instruction instr)
{
    switch(instr.code)
    {
    case MOVE:
    case ADDI:
    case LW:
        return instr.first_op;
    }
}

Value eval_static()
{
    Value ret;
    while(op_sp--)
    {
        switch (op_stack[op_sp].code) {
        case TConst:
            ret.integer = op_stack[op_sp].value.integer;
            break;
        default:
            break;
        }
    }
    ++op_sp;
    return ret;
}

ValueEntry pop()
{
    ValueEntry ret;
    Instruction instr;
    switch (op_stack[op_sp].code) {
    case TConst:
        ret = op_stack[op_sp];
        break;
    case TIdenttoval:
    {
        ValueEntry val = mips_identref[op_stack[op_sp].value.integer];
        switch (val.emplacement) {
        case MEM:
            instr.code = LA_;
            instr.first_op = select_tempreg();
            instr.second_op = name_from_identref(op_stack[op_sp].value.integer);
            code_instr[curCode++] = instr;
            instr.code = LW;
            instr.first_op = code_instr[curCode - 1].first_op;
            instr.second_op = code_instr[curCode - 1].first_op;
            instr.third_op = 0;
            code_instr[curCode++] = instr;
            ret.emplacement = TEMP;
            ret.value = instr.first_op;
            break;
        default:
            break;
        }
    }
    default:
        break;
    }
    return ret;
}

void eval_dynamic()
{
    Instruction instr;
    while(op_sp--)
    {
        switch (op_stack[op_sp].code) {
        case TReturnval:
            instr.code = MOVE;
            instr.first_op = $v0;
            instr.second_op = get_reg_from_instr(code_instr[curCode - 1]);
            code_instr[curCode++] = instr;
            instr.code = JR;
            instr.first_op = $ra;
            code_instr[curCode++] = instr;
            break;
        case LPLUS:
        {
            ValueEntry a, b;
            a = pop();
            b = pop();

            break;
        }
        default:
            break;
        }

    }
    ++op_sp;
}

void process_declaration()
{
    switch (tree[curTree++])
    {
        case TDeclid:
        {
            int identref = tree[curTree++], initref = tree[curTree++], N = tree[curTree++];
            switch (N)
            {
                case 0:
                    if (initref)
                    {
                        DataEntry data;
                        ValueEntry val;
                        process_expression();
                        data.value = eval_static();
                        data.name = name_from_identref(identref);
                        data.type = LINT;
                        if(level)
                        {
                            val.emplacement = STACK;
                            val.value.integer = val_sp++ * 4;
                        }
                        else
                        {
                            val.emplacement = MEM;
                            val.value.pointer = name_from_identref(identref);
                            data_entries[curData++] = data;
                        }
                        mips_identref[identref] = val;
                    }
                    break;
                case 1:
                    break;
                default:
                    break;
            }
            break;
        }
        case TFuncdef:
        {
            level = 1;
            process_function();
            level = 0;
            break;
        }
        default:
            break;
    }
}

void process_function()
{
    int identref = tree[curTree++], maxdisp = tree[curTree++];
    Instruction tmp;
    tmp.code = LABEL;
    tmp.first_op = name_from_identref(identref);
    code_instr[curCode++] = tmp;
    while (tree[curTree] == TDeclid)
        process_declaration();
    tmp.code = ADDI;
    tmp.first_op = $sp;
    tmp.second_op = $sp;
    tmp.third_op = val_sp * 4;
    code_instr[curCode++] = tmp;
    while (tree[curTree] != TEnd)
    {
        process_expression();
        eval_dynamic();
    }
    process_expression();
}

void process_expression()
{
    while (tree[curTree] != TExprend)
    {
        Operation oper;
        Instruction instr;
        switch (tree[curTree++])
        {
        case TConst:
            oper.code = TConst;
            oper.value.integer = tree[curTree++];
            op_stack[op_sp++] = oper;
            break;
        case TIdent:
            break;
        case TIdenttoval:
            oper.code = TIdenttoval;
            oper.value.integer = tree[curTree++];
            op_stack[op_sp++] = oper;
            break;
        case TReturn:
            instr.code = JR;
            instr.first_op = $ra;
            code_instr[curCode++] = instr;
            return;
        case TReturnval:
            oper.code = TReturnval;
            op_stack[op_sp++] = oper;
            break;
        case LPLUS:
            oper.code = LPLUS;
            op_stack[op_sp++] = oper;
        case TBegin:
            break;
        default:
            return;
        }
    }
    curTree++;
}

void compile_mips()
{
    int i;
    out_mips = fopen(outfile, "w");
    curTree = curData = curInit = curCode = 0;
    op_sp = val_sp = 0;
    do
    {
        level = 0;
        process_declaration();
        if(tree[curTree] == 0)
            break;
    } while(1);

    fprintf(output, ".data\n");
    for(i = 0; i < curData; i++)
        write_data(data_entries[i]);

    fprintf(output, ".text\n");
    for(i = 0; i < curInit; i++)
        write_op(initialise_instr[i]);
    for(i = 0; i < curCode; i++)
        write_op(code_instr[i]);

    fclose(output);
    return;
}
