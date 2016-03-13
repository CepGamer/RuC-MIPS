//  RuC2
//
//  Created by BSergei Bolotov on 03/Mar/2016.
//  Copyright (c) 2016 Sergei Bolotov. All rights reserved.
//

#include "global_vars.h"
#include "mips_defs.h"

char * outfile = "result.asm";
FILE * out_mips;

char * temp_label = "$label%d";
int curTempLabel;

int temp_regs_count = 10;
ValueEntry* temp_regs[10] = {0};
int saved_regs_count = 8;
ValueEntry* saved_regs[8] = {0};
Operation op_stack[100] = {0};
ValueEntry* val_stack[1000] = {0};
ValueEntry* static_vals[100] = {0};
ValueEntry all_values[1000] = {0};
ValueEntry* mips_identref[10000] = {0};
ValueEntry* declarations[10000] = {0};
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
    case $t5:
        return "$t5";
    case $t6:
        return "$t6";
    case $t7:
        return "$t7";
    case $t8:
        return "$t8";
    case $t9:
        return "$t9";
    case $sp:
        return "$sp";
    case $fp:
        return "$fp";
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
        if(instr.first_op < 0)
        {
            fprintf(output, "j\t");
            fprintf(output, temp_label, instr.second_op);
            fprintf(output, "\nnop\n");
        }
        else
            fprintf(output, "j\t%s\nnop\n", (char *)instr.first_op);
        break;
    case JAL:
        fprintf(output, "jal\t%s\nnop\n", (char *)instr.first_op);
        break;
    case JR:
        fprintf(output, "jr\t%s\nnop\n", reg_to_string(instr.first_op));
        break;
    case BEQZ:
        if(instr.second_op < 0)
        {
            fprintf(output, "beqz\t%s,", reg_to_string(instr.first_op));
            fprintf(output, temp_label, instr.third_op);
            fprintf(output, "\nnop\n");
        }
        else
            fprintf(output, "beqz\t%s,%s\nnop\n"
                    , reg_to_string(instr.first_op)
                    , (char *)instr.second_op);
        break;
    case LABEL:
        if(instr.first_op < 0)
        {
            fprintf(output, temp_label, instr.second_op);
            fprintf(output, ":\n");
        }
        else
            fprintf(output, "%s:\n", (char *)instr.first_op);
        break;
    case ADDI:
        fprintf(output, "addi\t%s,%s,%d\n"
                , reg_to_string(instr.first_op)
                , reg_to_string(instr.second_op)
                , instr.third_op);
        break;
    case ADDIU:
        fprintf(output, "addiu\t%s,%s,%d\n"
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
    case SLT:
        fprintf(output, "slt\t%s,%s,%s\n"
                , reg_to_string(instr.first_op)
                , reg_to_string(instr.second_op)
                , reg_to_string(instr.third_op));
        break;
    case SGT:
        fprintf(output, "sgt\t%s,%s,%s\n"
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
    case SW:
        fprintf(output, "sw\t%s,%d(%s)\n"
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

void process_declaration(int old_val_sp);
void process_function();
void process_expression();
void process_block();

void process_for();
void process_if();
void process_while();

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
            switch(val->emplacement)
            {
            case STATIC:
                instr.code = ADDIU;
                instr.first_op = val_to_reg(&t);
                instr.second_op = $0;
                instr.third_op = val->value.integer;
                code_instr[curCode++] = instr;
                break;
            case MEM:
                instr.code = LA_;
                instr.first_op = val_to_reg(&t);
                instr.second_op = (char*)val->value.pointer;
                code_instr[curCode++] = instr;
                instr.code = LW;
                instr.first_op = code_instr[curCode - 1].first_op;
                instr.second_op = code_instr[curCode - 1].first_op;
                instr.third_op = 0;
                code_instr[curCode++] = instr;
                break;
            case OTHER:
                instr.code = MOVE;
                instr.first_op = val_to_reg(&t);
                instr.second_op = val_to_reg(val);
                code_instr[curCode++] = instr;
                break;
            case STACK:
                instr.code = LW;
                instr.first_op = val_to_reg(&t);
                instr.second_op = $sp;
                instr.third_op = (val_sp - val->value.integer) * 4;
                code_instr[curCode++] = instr;
                break;
            default:
                break;
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
    int c;
    if(op_sp--)
        switch (c = op_stack[op_sp].code)
        {
        case TConst:
            ret = &all_values[all_values_sp++];
            ret->value = op_stack[op_sp].value;
            ret->emplacement = STATIC;
            break;
        case TIdenttoval:
            ret = mips_identref[op_stack[op_sp].value.integer];
            ret->emplacement = declarations[op_stack[op_sp].value.integer]->emplacement;
            ret->value = declarations[op_stack[op_sp].value.integer]->value;
            load_value(ret);
            break;
        case TIdent:
            ret = declarations[op_stack[op_sp].value.integer];
            break;
        case TCall2:
            instr.code = JAL;
            instr.first_op = op_stack[op_sp].value.pointer;
            code_instr[curCode++] = instr;
            ret = &all_values[all_values_sp++];
            ret->emplacement = OTHER;
            ret->value.integer = $v0;
            break;
        default:
            if ((c >= ASS && c <= DIVASS) ||
                (c >= ASSV && c <= DIVASSV) ||
                (c >= PLUSASSR && c <= DIVASSR) ||
                (c >= PLUSASSRV && c <= DIVASSRV) ||
                (c >= POSTINC && c <= DEC) ||
                (c >= POSTINCV && c <= DECV) ||
                (c >= POSTINCR && c <= DECR) ||
                (c >= POSTINCRV && c <= DECRV))
                ret = process_assign(c);
            else if ((c >= ASSAT && c <= DIVASSAT) ||
                (c >= ASSATV && c <= DIVASSATV) ||
                (c >= PLUSASSATR && c <= DIVASSATR) ||
                (c >= PLUSASSATRV && c <= DIVASSATRV) ||
                (c >= POSTINCAT && c <= DECAT) ||
                (c >= POSTINCATV && c <= DECATV) ||
                (c >= POSTINCATR && c <= DECATR) ||
                (c >= POSTINCATRV && c <= DECATRV))
                ret = process_assign_at(c);
            else if ((c >= LREM && c <= LDIV && c != LOGAND && c != LOGOR) ||
                (c >= EQEQR && c <= LDIVR))
                ret = process_binop(c);
            break;
        }
    else
        ++op_sp;
    return ret;
}

ValueEntry *eval_dynamic()
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
            case TReturn:
                //  Возврат адреса окна функции
                instr.code = LW;
                instr.first_op = $fp;
                instr.second_op = $sp;
                instr.third_op = val_sp * 4;
                code_instr[curCode++] = instr;
                //  Возврат адреса возврата
                instr.code = LW;
                instr.first_op = $ra;
                instr.second_op = $sp;
                instr.third_op = (val_sp - 1) * 4;
                code_instr[curCode++] = instr;
                if(val_sp)
                {
                    instr.code = ADDI;
                    instr.first_op = $sp;
                    instr.second_op = $sp;
                    instr.third_op = val_sp * 4;
                    code_instr[curCode++] = instr;
                }
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
    return prev;
}

void assign_to_ValueEntry(ValueEntry *to_ass, ValueEntry *new_val)
{
    Instruction instr;
    if(to_ass->emplacement == MEM)
    {
        ValueEntry *c = &all_values[all_values_sp++];
        c->emplacement = GARBAGE;
        load_value(c);
        instr.code = LA_;
        instr.first_op = val_to_reg(c);
        instr.second_op = (char*)to_ass->value.pointer;
        code_instr[curCode++] = instr;
        load_value(new_val);
        instr.code = SW;
        instr.first_op = val_to_reg(new_val);
        instr.second_op = val_to_reg(c);
        instr.third_op = 0;
        code_instr[curCode++] = instr;
    }
    else if (to_ass->emplacement == STACK)
    {
        load_value(new_val);
        instr.code = SW;
        instr.first_op = val_to_reg(new_val);
        instr.second_op = $sp;
        instr.third_op = (val_sp - to_ass->value.integer) * 4;
        code_instr[curCode++] = instr;
    }
}

ValueEntry *process_assign(int code)
{
    if ((code >= ASS && code <= DIVASS) ||
        (code >= PLUSASSR && code <= DIVASSR) ||
        (code >= ASSV && code <= DIVASSV) ||
        (code >= PLUSASSRV && code <= DIVASSRV))
    {
        ValueEntry *a = pop(), *b = pop();
        switch (code)
        {
        case ASS:
        case ASSV:
            assign_to_ValueEntry(b, a);
            return a;
            break;
        default:
            break;
        }
    }
    else if((code >= POSTINC && code <= DEC) ||
        (code >= POSTINCV && code <= DECV) ||
        (code >= POSTINCR && code <= DECR) ||
        (code >= POSTINCRV && code <= DECRV))
    {
        ValueEntry *a = pop(), *b;
        Instruction instr;
        b = &all_values[all_values_sp++];
        b->emplacement = a->emplacement;
        b->value = a->value;
        load_value(b);
        switch (code) {
        case INCV:
        case POSTINCV:
            instr.code = ADDIU;
            instr.first_op = val_to_reg(b);
            instr.second_op = val_to_reg(b);
            instr.third_op = 1;
            code_instr[curCode++] = instr;
            assign_to_ValueEntry(a, b);
            break;
        case DECV:
        case POSTDECV:
            instr.code = ADDIU;
            instr.first_op = val_to_reg(b);
            instr.second_op = val_to_reg(b);
            instr.third_op = -1;
            code_instr[curCode++] = instr;
            assign_to_ValueEntry(a, b);
            break;
        default:
            break;
        }
    }
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
        instr.first_op = instr.third_op = val_to_reg(a);
        instr.second_op = val_to_reg(b);
        code_instr[curCode++] = instr;
        b->emplacement = GARBAGE;
        ret = a;
        break;
    case LLT:
        instr.code = SLT;
        instr.first_op = instr.third_op = val_to_reg(a);
        instr.second_op = val_to_reg(b);
        code_instr[curCode++] = instr;
        b->emplacement = GARBAGE;
        ret = a;
        break;
    case LGT:
        instr.code = SGT;
        instr.first_op = instr.third_op = val_to_reg(a);
        instr.second_op = val_to_reg(b);
        code_instr[curCode++] = instr;
        b->emplacement = GARBAGE;
        ret = a;
        break;
    }
    return ret;
}

void process_for()
{
    int cond_label = curTempLabel++, incr_label = curTempLabel++, body_label = curTempLabel++, exit_label = curTempLabel++;
    curTree += 4;
    ValueEntry *tmp;
    Instruction instr;
    //  инициализация
    process_expression();
    eval_dynamic();
    //  проверка условия
    instr.code = LABEL;
    instr.first_op = -1;
    instr.second_op = cond_label;
    code_instr[curCode++] = instr;
    process_expression();
    tmp = eval_dynamic();
    load_value(tmp);
    instr.code = BEQZ;
    instr.first_op = val_to_reg(tmp);
    instr.second_op = -1;
    instr.third_op = exit_label;
    code_instr[curCode++] = instr;
    instr.code = J;
    instr.first_op = -1;
    instr.second_op = body_label;
    code_instr[curCode++] = instr;
    //  инкремент
    instr.code = LABEL;
    instr.first_op = -1;
    instr.second_op = incr_label;
    code_instr[curCode++] = instr;
    process_expression();
    eval_dynamic();
    instr.code = J;
    instr.first_op = -1;
    instr.second_op = cond_label;
    code_instr[curCode++] = instr;
    //  Тело цикла
    instr.code = LABEL;
    instr.first_op = -1;
    instr.second_op = body_label;
    code_instr[curCode++] = instr;
    process_block();

    instr.code = J;
    instr.first_op = -1;
    instr.second_op = incr_label;
    code_instr[curCode++] = instr;
    //  Выход из цикла
    instr.code = LABEL;
    instr.first_op = -1;
    instr.second_op = exit_label;
    code_instr[curCode++] = instr;
}

void process_if()
{
    int else_ref = tree[++curTree];
    int else_label = curTempLabel++, exit_label = curTempLabel++;
    ValueEntry *cond;
    Instruction instr;
    ++curTree;
    //  условие
    process_expression();
    cond = eval_dynamic();
    load_value(cond);
    instr.code = BEQZ;
    instr.first_op = val_to_reg(cond);
    instr.second_op = -1;
    instr.third_op = else_ref ? else_label : exit_label;
    code_instr[curCode++] = instr;
    //  если условие выполнено
    process_block();
    //  иначе
    if(else_ref)
    {
        instr.code = LABEL;
        instr.first_op = -1;
        instr.second_op = else_label;
        code_instr[curCode++] = instr;
        process_block();
    }
    instr.code = LABEL;
    instr.first_op = -1;
    instr.second_op = exit_label;
    code_instr[curCode++] = instr;
}

void process_while()
{

}

void process_declaration(int old_val_sp)
{
    switch (tree[curTree++])
    {
        case TDeclid:
        {
            int identref = tree[curTree++], initref = tree[curTree++], N = tree[curTree++];
            switch (N)
            {
                case 0:
                {
                    DataEntry data;
                    ValueEntry *val = &all_values[all_values_sp++];
                    if(initref)
                    {
                        process_expression();
                        data.value = eval_static();
                    }
                    data.name = name_from_identref(identref);
                    data.type = LINT;
                    if(level)
                    {
                        if (initref)
                        {
                            Instruction instr;
                            instr.code = ADDIU;
                            instr.first_op = $t0;
                            instr.second_op = $0;
                            instr.third_op = data.value.integer;
                            code_instr[curCode++] = instr;
                            instr.code = SW;
                            instr.first_op = $t0;
                            instr.second_op = $sp;
                            instr.third_op = -(val_sp - old_val_sp) * 4;
                            code_instr[curCode++] = instr;
                        }
                        val->emplacement = STACK;
                        val->value.integer = val_sp++;
                    }
                    else
                    {
                        val->emplacement = MEM;
                        val->value.pointer = name_from_identref(identref);
                        data_entries[curData++] = data;
                    }
                    mips_identref[identref] = val;
                    declarations[identref] = &all_values[all_values_sp++];
                    declarations[identref]->emplacement = val->emplacement;
                    declarations[identref]->value = val->value;
                    break;
                }
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
    curTree++;
    val_sp += 2;

    //  Определения
    while (tree[curTree] == TDeclid)
        process_declaration(0);

    //  Сохранение указателя на окно функции и адреса возврата
    tmp.code = ADDIU;
    tmp.first_op = $sp;
    tmp.second_op = $sp;
    tmp.third_op = -val_sp * 4;
    code_instr[curCode++] = tmp;
    tmp.code = SW;
    tmp.first_op = $fp;
    tmp.second_op = $sp;
    tmp.third_op = val_sp * 4;
    code_instr[curCode++] = tmp;
    tmp.code = SW;
    tmp.first_op = $ra;
    tmp.second_op = $sp;
    tmp.third_op = (val_sp - 1) * 4;
    code_instr[curCode++] = tmp;
    //  Запись адреса нового окна функции
    tmp.code = MOVE;
    tmp.first_op = $fp;
    tmp.second_op = $sp;
    code_instr[curCode++] = tmp;

    //  Выражения
    while (tree[curTree] != TEnd)
    {
        process_expression();
        eval_dynamic();
    }
    val_sp = 0;
}

void process_expression()
{
    while (tree[curTree] != TExprend && tree[curTree] != TEnd)
    {
        Operation oper;
        int c;
        switch (c = tree[curTree++])
        {
        case TConst:
            oper.code = TConst;
            oper.value.integer = tree[curTree++];
            op_stack[op_sp++] = oper;
            break;
        case TIdent:
            oper.code = TIdent;
            oper.value.integer = tree[curTree++];
            op_stack[op_sp++] = oper;
            break;
        case TIdenttoval:
            oper.code = TIdenttoval;
            oper.value.integer = tree[curTree++];
            op_stack[op_sp++] = oper;
            break;
        case TFor:
            process_for();
            break;
        case TIf:
            process_if();
            break;
        case TWhile:
        case TCall1:
            curTree++;
            break;
        case TCall2:
            oper.code = TCall2;
            oper.value.pointer = name_from_identref(-tree[curTree++]);
            op_sp = 0;
            op_stack[op_sp++] = oper;
            break;
        case TPrint:
            oper.code = TPrint;
            oper.value.integer = tree[curTree++];
            op_stack[op_sp++] = oper;
            return;
            break;
        case TReturn:
            oper.code = TReturn;
            op_stack[op_sp++] = oper;
            return;
        case TReturnval:
            oper.code = TReturnval;
            op_stack[op_sp++] = oper;
            break;
        case TBegin:
            break;
        case LOGAND:
        case LOGOR:
            break;
        default:
            if ((c >= ASS && c <= DIVASS) ||
                (c >= ASSV && c <= DIVASSV) ||
                (c >= PLUSASSR && c <= DIVASSR) ||
                (c >= PLUSASSRV && c <= DIVASSRV) ||
                (c >= POSTINC && c <= DEC) ||
                (c >= POSTINCV && c <= DECV) ||
                (c >= POSTINCR && c <= DECR) ||
                (c >= POSTINCRV && c <= DECRV) ||
                (c >= ASSAT && c <= DIVASSAT) ||
                (c >= ASSATV && c <= DIVASSATV) ||
                (c >= PLUSASSATR && c <= DIVASSATR) ||
                (c >= PLUSASSATRV && c <= DIVASSATRV) ||
                (c >= POSTINCAT && c <= DECAT) ||
                (c >= POSTINCATV && c <= DECATV) ||
                (c >= POSTINCATR && c <= DECATR) ||
                (c >= POSTINCATRV && c <= DECATRV) ||
                (c >= LREM && c <= LDIV && c != LOGAND && c != LOGOR) ||
                (c >= EQEQR && c <= LDIVR))
            {
                oper.code = c;
                op_stack[op_sp++] = oper;
                break;
            }
            break;
        }
    }
    if(tree[curTree] == TExprend)
        curTree++;
}

void process_block()
{
    int old_val_sp = val_sp;
    Instruction instr;
    if(tree[curTree] == TBegin)
    {
        ++curTree;
        while (tree[curTree] == TDeclid)
            process_declaration(old_val_sp);
        //  рост стека
        if(val_sp != old_val_sp)
        {
            instr.code = ADDIU;
            instr.first_op = $sp;
            instr.second_op = $sp;
            instr.third_op = -(val_sp - old_val_sp) * 4;
            code_instr[curCode++] = instr;
        }
        while (tree[curTree] != TEnd)
        {
            process_expression();
            eval_dynamic();
        }
    }
    else
    {
        process_expression();
        eval_dynamic();
    }
    if(val_sp != old_val_sp)
    {
        instr.code = ADDIU;
        instr.first_op = $sp;
        instr.second_op = $sp;
        instr.third_op = (val_sp - old_val_sp) * 4;
        code_instr[curCode++] = instr;
        val_sp = old_val_sp;
    }
}

void compile_mips()
{
    int i;
    Instruction gotomain;
    out_mips = fopen(outfile, "w");
    curTree = curData = curInit = curCode = 0;
    curTempLabel = 0;
    op_sp = val_sp = all_values_sp = 0;
    do
    {
        level = 0;
        process_declaration(0);
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
