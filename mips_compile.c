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

int temp_regs_count = 10;
ValueEntry* temp_regs[10] = {0};
int saved_regs_count = 8;
ValueEntry* saved_regs[8] = {0};
Operation op_stack[100] = {0};
ValueEntry* val_stack[1000] = {0};
ValueEntry* static_vals[100] = {0};
ValueEntry all_values[1000] = {0};
ValueEntry* mips_identref[10000] = {0};
int op_sp, val_sp, all_values_sp;

DataEntry data_entries[1000];
Instruction initialise_instr[1000], code_instr[10000];

int curTree, curData, curInit, curCode;
int level;

char * name_from_identref(int identref)
{
    return reprtab + ((identab[identref + 1] + 2 < 28 ? 8 : identab[identref + 1]) + 2);
}

Registers val_to_reg(ValueEntry *val)
{
    switch(val->emplacement)
    {
    case TEMP:
        if(val->value.integer < 8)
            return $t0 + val->value.integer;
        else
            return $t8 + val->value.integer - 8;
        break;
    case OTHER:
        return val->value.integer;
    }
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
    case $a0:
        return "$a0";
    case $a1:
        return "$a1";
    case $t0:
        return "$t0";
    case $t1:
        return "$t1";
    case $t2:
        return "$t2";
    case $t3:
        return "$t3";
    case $t4:
        return "$t4";
    case $sp:
        return "$sp";
    case $ra:
        return "$ra";
    default:
        return "";
    }
}

void write_instr(Instruction instr)
{
    switch (instr.code) {
    case J:
        fprintf(output, "j\t%s\nnop\n", (char *)instr.first_op);
        break;
    case JAL:
        fprintf(output, "jal\t%s\nnop\n", (char *)instr.first_op);
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
    case ADD:
        fprintf(output, "add\t%s,%s,%s\n"
                , reg_to_string(instr.first_op)
                , reg_to_string(instr.second_op)
                , reg_to_string(instr.third_op));
        break;
    case MOVE:
        fprintf(output, "move\t%s,%s\n"
                , reg_to_string(instr.first_op)
                , reg_to_string(instr.second_op));
        break;
    case LI_:
        fprintf(output, "li\t%s,%d\n"
                , reg_to_string(instr.first_op)
                , instr.second_op);
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
    case SYSCALL:
        fprintf(output, "syscall\n");
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

ValueEntry *process_assign(int code);
ValueEntry *process_assign_at(int code);
ValueEntry *process_binop(int code);

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

void load_value(ValueEntry *val)
{
    int i;
    static int nxt = 0;
    Instruction instr;
    if(val->emplacement == TEMP || val->emplacement == SAVED)
        return;
    for(i = 0; i < temp_regs_count; ++i)
        if(!temp_regs[i] || temp_regs[i]->emplacement != TEMP)
        {
            ValueEntry t;
            t.emplacement = TEMP;
            t.value.integer = i;
            if(val->emplacement == STATIC)
            {
                instr.code = ADDI;
                instr.first_op = val_to_reg(&t);
                instr.second_op = $0;
                instr.third_op = val->value.integer;
                code_instr[curCode++] = instr;
            } else if(val->emplacement == MEM)
            {
                instr.code = LA_;
                instr.first_op = val_to_reg(&t);
                instr.second_op = (char*)val->value.pointer;
                code_instr[curCode++] = instr;
                instr.code = LW;
                instr.first_op = code_instr[curCode - 1].first_op;
                instr.second_op = code_instr[curCode - 1].first_op;
                instr.third_op = 0;
                code_instr[curCode++] = instr;
            }
            else if(val->emplacement == OTHER)
            {
                instr.code = MOVE;
                instr.first_op = val_to_reg(&t);
                instr.second_op = val_to_reg(val);
                code_instr[curCode++] = instr;
            }
            val->emplacement = TEMP;
            val->value.integer = i;
            temp_regs[i] = val;
            return;
        }
    temp_regs[nxt]->emplacement = STACK;
    temp_regs[nxt]->value.integer = val_sp;
    //  TODO
    //  Add instruction
//    instr.code =
    val_stack[val_sp++] = temp_regs[nxt];
    val->emplacement = TEMP;
    val->value.integer = nxt;
    temp_regs[nxt++] = val;
}

ValueEntry* pop()
{
    ValueEntry* ret;
    Instruction instr;
    if(op_sp--)
        switch (op_stack[op_sp].code)
        {
        case TConst:
            ret = &all_values[all_values_sp++];
            ret->value = op_stack[op_sp].value;
            ret->emplacement = STATIC;
            break;
        case TIdenttoval:
        {
            ret = mips_identref[op_stack[op_sp].value.integer];
            switch (ret->emplacement)
            {
            case MEM:
                load_value(ret);
                break;
            default:
                break;
            }
            break;
        }
        case TCall2:
            instr.code = JAL;
            instr.first_op = op_stack[op_sp].value.pointer;
            code_instr[curCode++] = instr;
            ret = &all_values[all_values_sp++];
            ret->emplacement = OTHER;
            ret->value.integer = $v0;
            break;
        default:
            break;
        }
    else
        ++op_sp;
    return ret;
}

void eval_dynamic()
{
    Instruction instr;
    static ValueEntry *prev;
    while(op_sp--)
    {
        int c = op_stack[op_sp].code;
        if ((c >= ASS && c <= DIVASS) ||
            (c >= ASSV && c <= DIVASSV) ||
            (c >= PLUSASSR && c <= DIVASSR) ||
            (c >= PLUSASSRV && c <= DIVASSRV) ||
            (c >= POSTINC && c <= DEC) ||
            (c >= POSTINCV && c <= DECV) ||
            (c >= POSTINCR && c <= DECR) ||
            (c >= POSTINCRV && c <= DECRV))
            prev = process_assign(c);
        else if ((c >= ASSAT && c <= DIVASSAT) ||
            (c >= ASSATV && c <= DIVASSATV) ||
            (c >= PLUSASSATR && c <= DIVASSATR) ||
            (c >= PLUSASSATRV && c <= DIVASSATRV) ||
            (c >= POSTINCAT && c <= DECAT) ||
            (c >= POSTINCATV && c <= DECATV) ||
            (c >= POSTINCATR && c <= DECATR) ||
            (c >= POSTINCATRV && c <= DECATRV))
            prev = process_assign_at(c);
        else if ((c >= LREM && c <= LDIV && c != LOGAND && c != LOGOR) ||
            (c >= EQEQR && c <= LDIVR))
            prev = process_binop(c);
        else
            switch (c)
            {
            case TReturnval:
                instr.code = MOVE;
                instr.first_op = $v0;
                instr.second_op = val_to_reg(prev);
                code_instr[curCode++] = instr;
                instr.code = JR;
                instr.first_op = $ra;
                code_instr[curCode++] = instr;
                break;
            case TPrint:
                switch (op_stack[op_sp].value.integer)
                {
                case LINT:
                    prev = pop();
                    load_value(prev);
                    instr.code = LI_;
                    instr.first_op = $v0;
                    instr.second_op = 1;
                    code_instr[curCode++] = instr;
                    instr.code = MOVE;
                    instr.first_op = $a0;
                    instr.second_op = val_to_reg(prev);
                    code_instr[curCode++] = instr;
                    instr.code = SYSCALL;
                    code_instr[curCode++] = instr;
                    break;
                default:
                    break;
                }
                break;
            default:
                ++op_sp;
                prev = pop();
                break;
            }
    }
    ++op_sp;
}

ValueEntry *process_assign(int code)
{

}

ValueEntry *process_assign_at(int code)
{

}

ValueEntry *process_binop(int code)
{
    ValueEntry *a, *b, *ret;
    Instruction instr;
    a = pop();
    b = pop();
    load_value(a);
    load_value(b);
    switch (code)
    {
    case LPLUS:
        instr.code = ADD;
        instr.first_op = instr.second_op = val_to_reg(a);
        instr.third_op = val_to_reg(b);
        code_instr[curCode++] = instr;
        b->emplacement = GARBAGE;
        ret = a;
        break;
    }
    return ret;
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
                        ValueEntry *val = &all_values[all_values_sp++];
                        process_expression();
                        data.value = eval_static();
                        data.name = name_from_identref(identref);
                        data.type = LINT;
                        if(level)
                        {
                            val->emplacement = STACK;
                            val->value.integer = val_sp++ * 4;
                        }
                        else
                        {
                            val->emplacement = MEM;
                            val->value.pointer = name_from_identref(identref);
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
            curTree++;
            break;
        case TIdenttoval:
            oper.code = TIdenttoval;
            oper.value.integer = tree[curTree++];
            op_stack[op_sp++] = oper;
            break;
        case TCall1:
            curTree++;
            break;
        case TCall2:
            oper.code = TCall2;
            oper.value.pointer = name_from_identref(-tree[curTree++]);
            op_stack[op_sp++] = oper;
            break;
        case TPrint:
            oper.code = TPrint;
            oper.value.integer = tree[curTree++];
            op_stack[op_sp++] = oper;
            return;
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
    Instruction gotomain;
    out_mips = fopen(outfile, "w");
    curTree = curData = curInit = curCode = 0;
    op_sp = val_sp = all_values_sp = 0;
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
        write_instr(initialise_instr[i]);
    gotomain.code = J;
    gotomain.first_op = name_from_identref(0);
    write_instr(gotomain);
    for(i = 0; i < curCode; i++)
        write_instr(code_instr[i]);

    fclose(output);
    return;
}
