//  RuC2
//
//  Created by Sergei Bolotov on 03/Mar/2016.
//  Copyright (c) 2016 Sergei Bolotov. All rights reserved.
//

#include "global_vars.h"
#include "mips_defs.h"

char *outfile = "result.asm";
FILE *out_mips;

char *temp_label = "$label%d";
int curTempLabel;

/* регистры аргументов */
int arg_regs_count = 4;
ValueEntry *arg_regs[4];
/* временные регистры */
int temp_regs_count = 10;
ValueEntry *temp_regs[10] = {0};
ValueDiff old_temp_regs[10] = {0};
/* сохранённые регистры */
int saved_regs_count = 8;
ValueEntry *saved_regs[8] = {0};

/* стэк операций */
Operation op_stack[100] = {0};

/* пул значений */
ValueEntry all_values[10000] = {0};
/* содержит текущее состояние переменной */
ValueEntry *mips_identref[10000] = {0};
/* содержит сведения об определении переменной */
ValueEntry *declarations[10000] = {0};

/* указатели на стэки */
int op_sp, val_sp, all_values_sp;

/* имя переменной на печать */
char temp_name[100];

IdentEntry data_entries[1000];
Instruction initialise_instr[1000], code_instr[10000];

IdentDiff changed_ids[1000];
ValueDiff changed_vals[1000];

int curTree, curData, curInit, curCode, curChangedId, curChangedVal;
int level;
int track_changes;

/* различные сохранённые данные о программе */
int had_func_call;
int cur_type;
int continue_label = -1, break_label = -1;

int fp_codes[100];
int fp_codes_ptr;

int propagate_constants = 1;
int delayed_slot = 1;

void assign_ValueEntry(ValueEntry *ptr, ValueEntry *to_ass)
{
    ptr->emplacement = to_ass->emplacement;
    ptr->flags = to_ass->flags;
    ptr->value = to_ass->value;
}

ValueEntry *copy_value_entry(ValueEntry *entry)
{
    ValueEntry *ret = &all_values[all_values_sp++];

    assign_ValueEntry(ret, entry);
    return ret;
}

Instruction create_instr(Instructions code, int first_op, int second_op, int third_op)
{
    Instruction ret;
    ret.code = code;
    ret.first_op = first_op;
    ret.second_op = second_op;
    ret.third_op = third_op;
    return ret;
}

IdentDiff create_ident_diff(int ident, ValueEntry old)
{
    IdentDiff diff;
    diff.ident = ident;
    diff.old = old;
    return diff;
}

ValueDiff create_value_diff(ValueEntry *ident, ValueEntry old)
{
    ValueDiff diff;
    diff.ident = ident;
    diff.old = old;
    return diff;
}

void save_temp()
{
    int i;
    for(i = 0; i < temp_regs_count; ++i)
        if(temp_regs[i])
            old_temp_regs[i] = create_value_diff(temp_regs[i], *temp_regs[i]);
}

void restore_temp()
{
    int i;
    for(i = 0; i < temp_regs_count; ++i)
    {
        temp_regs[i] = old_temp_regs[i].ident;
        if(temp_regs[i])
            assign_ValueEntry(temp_regs[i], &old_temp_regs[i].old);
    }
}

void restore_from_diff()
{
    restore_temp();
    while(curChangedId--)
    {
        assign_ValueEntry(mips_identref[changed_ids[curChangedId].ident], &changed_ids[curChangedId].old);
        if(changed_ids[curChangedId].old.emplacement == TEMP)
            temp_regs[changed_ids[curChangedId].old.value.integer] = mips_identref[changed_ids[curChangedId].ident];
    }
    curChangedId = 0;
}

void start_track_changes()
{
    track_changes = 1;
    save_temp();
}

void stop_track_changes()
{
    track_changes = 0;
}

int type_from_identref(int identref)
{
    return identab[identref + 2];
}

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
    case ARG:
        return val->value.integer + $a0;
    case OTHER:
        return val->value.integer;
    default:
        break;
    }
    return $0;
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
    case $a2:
        return "$a2";
    case $a3:
        return "$a3";
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

void fill_temp_name(int *pointer)
{
    int i = 0;
    while(temp_name[i++] = *pointer++);
}

void drop_temp_regs()
{
    int i = 0;
    for(i; i < temp_regs_count; ++i)
        if(temp_regs[i])
            temp_regs[i]->emplacement = GARBAGE;
}

void write_instr(Instruction instr)
{
    switch (instr.code) {
    case DELETED:
        break;
    case J:
        if(instr.first_op < 0)
        {
            fprintf(output, "\tj\t");
            fprintf(output, temp_label, instr.second_op);
            fprintf(output, "\nnop\n");
        }
        else
        {
            fill_temp_name(instr.first_op);
            fprintf(output, "\tj\t%s\nnop\n", temp_name);
        }
        break;
    case JAL:
        fill_temp_name(instr.first_op);
        fprintf(output, "\tjal\t%s\nnop\n", temp_name);
        break;
    case JR:
        fprintf(output, "\tjr\t%s\nnop\n", reg_to_string(instr.first_op));
        break;
    case JALR:
        fprintf(output, "\tjalr\t%s\nnop\n", reg_to_string(instr.first_op));
        break;
    case BEQ:
        if(instr.third_op < 0)
        {
            fprintf(output, "\tbeq\t%s,%s,", reg_to_string(instr.first_op), reg_to_string(instr.second_op));
            fprintf(output, temp_label, -instr.third_op);
            fprintf(output, "\nnop\n");
        }
        else
            fprintf(output, "\tbeq\t%s,%s,%s\nnop\n"
                    , reg_to_string(instr.first_op)
                    , reg_to_string(instr.second_op)
                    , (char *)instr.third_op);
        break;
    case BEQI:
        if(instr.third_op < 0)
        {
            fprintf(output, "\tbeq\t%s,%d,", reg_to_string(instr.first_op), instr.second_op);
            fprintf(output, temp_label, instr.third_op);
            fprintf(output, "\nnop\n");
        }
        else
            fprintf(output, "\tbeq\t%s,%d,%s\nnop\n"
                    , reg_to_string(instr.first_op)
                    , instr.second_op
                    , (char *)instr.third_op);
        break;
    case BEQZ:
        if(instr.second_op < 0)
        {
            fprintf(output, "\tbeqz\t%s,", reg_to_string(instr.first_op));
            fprintf(output, temp_label, instr.third_op);
            fprintf(output, "\nnop\n");
        }
        else
            fprintf(output, "\tbeqz\t%s,%s\nnop\n"
                    , reg_to_string(instr.first_op)
                    , (char *)instr.second_op);
        break;
    case BNEZ:
        if(instr.second_op < 0)
        {
            fprintf(output, "\tbnez\t%s,", reg_to_string(instr.first_op));
            fprintf(output, temp_label, instr.third_op);
            fprintf(output, "\nnop\n");
        }
        else
            fprintf(output, "\tbnez\t%s,%s\nnop\n"
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
        {
            fill_temp_name(instr.first_op);
            fprintf(output, "%s:\n", temp_name);
        }
        break;
    case ADD:
        fprintf(output, "\tadd\t%s,%s,%s\n"
                , reg_to_string(instr.first_op)
                , reg_to_string(instr.second_op)
                , reg_to_string(instr.third_op));
        break;
    case ADDU:
        fprintf(output, "\taddu\t%s,%s,%s\n"
                , reg_to_string(instr.first_op)
                , reg_to_string(instr.second_op)
                , reg_to_string(instr.third_op));
        break;
    case SUB:
        fprintf(output, "\tsub\t%s,%s,%s\n"
                , reg_to_string(instr.first_op)
                , reg_to_string(instr.second_op)
                , reg_to_string(instr.third_op));
        break;
    case SUBU:
        fprintf(output, "\tsubu\t%s,%s,%s\n"
                , reg_to_string(instr.first_op)
                , reg_to_string(instr.second_op)
                , reg_to_string(instr.third_op));
        break;
    case SUBIU:
        fprintf(output, "\tsubiu\t%s,%s,%d\n"
                , reg_to_string(instr.first_op)
                , reg_to_string(instr.second_op)
                , instr.third_op);
        break;
    case ADDI:
        fprintf(output, "\taddi\t%s,%s,%d\n"
                , reg_to_string(instr.first_op)
                , reg_to_string(instr.second_op)
                , instr.third_op);
        break;
    case ADDIU:
        fprintf(output, "\taddiu\t%s,%s,%d\n"
                , reg_to_string(instr.first_op)
                , reg_to_string(instr.second_op)
                , instr.third_op);
        break;
    case MUL:
        fprintf(output, "\tmul\t%s,%s,%s\n"
                , reg_to_string(instr.first_op)
                , reg_to_string(instr.second_op)
                , reg_to_string(instr.third_op));
        break;
    case DIV:
    case REM:
        fprintf(output, "\tdiv\t%s,%s,%s\n"
                , reg_to_string($0)
                , reg_to_string(instr.second_op)
                , reg_to_string(instr.third_op));
        fprintf(output, "\tteq\t%s,%s\n"
                , reg_to_string(instr.third_op)
                , reg_to_string($0));
        if(instr.code == REM)
            fprintf(output, "\tmfhi\t%s\n"
                    , reg_to_string(instr.first_op));
        else
            fprintf(output, "\tmflo\t%s\n"
                    , reg_to_string(instr.first_op));
        break;
    case DIVI:
    case REMI:
        fprintf(output, "\tdiv\t%s,%s,%d\n"
                , reg_to_string($0)
                , reg_to_string(instr.second_op)
                , instr.third_op);
        fprintf(output, "\tteq\t%d,%s\n"
                , instr.third_op
                , reg_to_string($0));
        if(instr.code == REM)
            fprintf(output, "\tmfhi\t%s\n"
                    , reg_to_string(instr.first_op));
        else
            fprintf(output, "\tmflo\t%s\n"
                    , reg_to_string(instr.first_op));
        break;
    case OR:
        fprintf(output, "\tor\t%s,%s,%s\n"
                , reg_to_string(instr.first_op)
                , reg_to_string(instr.second_op)
                , reg_to_string(instr.third_op));
        break;
    case ORI:
        fprintf(output, "\tori\t%s,%s,%d\n"
                , reg_to_string(instr.first_op)
                , reg_to_string(instr.second_op)
                , instr.third_op);
        break;
    case XOR:
        fprintf(output, "\txor\t%s,%s,%s\n"
                , reg_to_string(instr.first_op)
                , reg_to_string(instr.second_op)
                , reg_to_string(instr.third_op));
        break;
    case XORI:
        fprintf(output, "\txori\t%s,%s,%d\n"
                , reg_to_string(instr.first_op)
                , reg_to_string(instr.second_op)
                , instr.third_op);
        break;
    case AND:
        fprintf(output, "\tand\t%s,%s,%s\n"
                , reg_to_string(instr.first_op)
                , reg_to_string(instr.second_op)
                , reg_to_string(instr.third_op));
        break;
    case ANDI:
        fprintf(output, "\tandi\t%s,%s,%d\n"
                , reg_to_string(instr.first_op)
                , reg_to_string(instr.second_op)
                , instr.third_op);
        break;
    case SLL:
        fprintf(output, "\tsll\t%s,%s,%d\n"
                , reg_to_string(instr.first_op)
                , reg_to_string(instr.second_op)
                , instr.third_op);
        break;
    case SLLV:
        fprintf(output, "\tsllv\t%s,%s,%s\n"
                , reg_to_string(instr.first_op)
                , reg_to_string(instr.second_op)
                , reg_to_string(instr.third_op));
        break;
    case SRA:
        fprintf(output, "\tsra\t%s,%s,%d\n"
                , reg_to_string(instr.first_op)
                , reg_to_string(instr.second_op)
                , instr.third_op);
        break;
    case SRAV:
        fprintf(output, "\tsrav\t%s,%s,%s\n"
                , reg_to_string(instr.first_op)
                , reg_to_string(instr.second_op)
                , reg_to_string(instr.third_op));
        break;
    case SLT:
        fprintf(output, "\tslt\t%s,%s,%s\n"
                , reg_to_string(instr.first_op)
                , reg_to_string(instr.second_op)
                , reg_to_string(instr.third_op));
        break;
    case SGT:
        fprintf(output, "\tsgt\t%s,%s,%s\n"
                , reg_to_string(instr.first_op)
                , reg_to_string(instr.second_op)
                , reg_to_string(instr.third_op));
        break;
    case SLE:
        fprintf(output, "\tsle\t%s,%s,%s\n"
                , reg_to_string(instr.first_op)
                , reg_to_string(instr.second_op)
                , reg_to_string(instr.third_op));
        break;
    case SGE:
        fprintf(output, "\tsge\t%s,%s,%s\n"
                , reg_to_string(instr.first_op)
                , reg_to_string(instr.second_op)
                , reg_to_string(instr.third_op));
        break;
    case SEQ:
        fprintf(output, "\tseq\t%s,%s,%s\n"
                , reg_to_string(instr.first_op)
                , reg_to_string(instr.second_op)
                , reg_to_string(instr.third_op));
        break;
    case SEQI:
        fprintf(output, "\tseq\t%s,%s,%d\n"
                , reg_to_string(instr.first_op)
                , reg_to_string(instr.second_op)
                , instr.third_op);
        break;
    case SNE:
        fprintf(output, "\tsne\t%s,%s,%s\n"
                , reg_to_string(instr.first_op)
                , reg_to_string(instr.second_op)
                , reg_to_string(instr.third_op));
        break;
    case SNEI:
        fprintf(output, "\tsne\t%s,%s,%d\n"
                , reg_to_string(instr.first_op)
                , reg_to_string(instr.second_op)
                , instr.third_op);
        break;
    case MOVE:
        fprintf(output, "\tmove\t%s,%s\n"
                , reg_to_string(instr.first_op)
                , reg_to_string(instr.second_op));
        break;
    case NEGU:
        fprintf(output, "\tnegu\t%s,%s\n"
                , reg_to_string(instr.first_op)
                , reg_to_string(instr.second_op));
        break;
    case LI_:
        fprintf(output, "\tli\t%s,%d\n"
                , reg_to_string(instr.first_op)
                , instr.second_op);
        break;
    case LA_:
        fill_temp_name(instr.second_op);
        fprintf(output, "\tla\t%s,%s\n"
                , reg_to_string(instr.first_op)
                , temp_name);
        break;
    case LW:
        fprintf(output, "\tlw\t%s,%d(%s)\n"
                , reg_to_string(instr.first_op)
                , instr.third_op
                , reg_to_string(instr.second_op));
        break;
    case SW:
        fprintf(output, "\tsw\t%s,%d(%s)\n"
                , reg_to_string(instr.first_op)
                , instr.third_op
                , reg_to_string(instr.second_op));
        break;
    case SYSCALL:
        fprintf(output, "\tsyscall\n");
        break;
    default:
        break;
    }
}

void write_data(IdentEntry data)
{
    fill_temp_name(data.name);
    fprintf(output, "%s:\t", temp_name);
    switch(data.type)
    {
    int i;
    case LINT:
        fprintf(output, ".word\t%d\n", data.value.integer);
        break;
    case LFLOAT:
        break;
    case LCHAR:
        break;
    case ROWOFINT:
    case ROWROWOFINT:
        if(data.value.pointer)
        {
            fprintf(output, ".word\t");
            for(i = 0; i < data.size - 1; ++i)
                fprintf(output, "%d, ", ((Value *)data.value.pointer)[i].integer);
            fprintf(output, "%d\n", ((Value *)data.value.pointer)[data.size - 1].integer);
        }
        else
            fprintf(output, ".space\t%d\n", data.size * sizeof(int));
        break;
    case ROWOFFLOAT:
    case ROWROWOFFLOAT:
        break;
    case ROWOFCHAR:
    case ROWROWOFCHAR:
        break;
    }
}

void process_declaration(int old_val_sp);
void process_function();
void process_expression();
void process_block();

void mark_block();

void process_for();
void process_if();
void process_while();
void process_do_while();
void process_switch();

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
    default:
        break;
    }
    return $0;
}

Value ret_val_int(Value a, Value b, int code)
{
    Value ret;
    switch (code) {
    case LPLUS:
    case PLUSASS:
    case PLUSASSV:
    case PLUSASSAT:
    case PLUSASSATV:
        ret.integer = b.integer + a.integer;
        break;
    case LMINUS:
    case MINUSASS:
    case MINUSASSV:
    case MINUSASSAT:
    case MINUSASSATV:
        ret.integer = b.integer - a.integer;
        break;
    //  TODO продолжить
    case LOGAND:
        ret.integer = b.integer && a.integer;
        break;
    case LOGOR:
        ret.integer = b.integer || a.integer;
        break;
    case LSHL:
        ret.integer = b.integer << a.integer;
        break;
    case LSHR:
        ret.integer = b.integer >> a.integer;
        break;
    case LAND:
        ret.integer = b.integer & a.integer;
        break;
    case LEXOR:
        ret.integer = b.integer ^ a.integer;
        break;
    case LOR:
        ret.integer = b.integer | a.integer;
        break;
    case LMULT:
        ret.integer = b.integer * a.integer;
        break;
    case LDIV:
        ret.integer = b.integer / a.integer;
        break;
    case LREM:
        ret.integer = b.integer % a.integer;
        break;
    case EQEQ:
        ret.integer = b.integer == a.integer;
        break;
    case NOTEQ:
        ret.integer = b.integer != a.integer;
        break;
    case LLT:
        ret.integer = b.integer < a.integer;
        break;
    case LGT:
        ret.integer = b.integer > a.integer;
        break;
    case LLE:
        ret.integer = b.integer <= a.integer;
        break;
    case LGE:
        ret.integer = b.integer >= a.integer;
        break;
    default:
        break;
    }
    return ret;
}

Value eval_static()
{
    Value ret;
    int code;
    if(!op_sp)
        return ret;
    switch (code = op_stack[--op_sp].code)
    {
    case TConst:
        ret.integer = op_stack[op_sp].value.integer;
        break;
    default:
        ret = ret_val_int(eval_static(), eval_static(), code);
        break;
    }
    return ret;
}

void load_value(ValueEntry *val)
{
    int i = 0;
    static int nxt = 0;
    if(val->emplacement == TEMP || val->emplacement == SAVED || val->emplacement == IDENT_)
        return;

cycle:
    for(i; i < temp_regs_count; ++i)
        if(!temp_regs[i] || temp_regs[i]->emplacement != TEMP)
        {
            ValueEntry t;
            t.emplacement = TEMP;
            t.value.integer = i;
            switch(val->emplacement)
            {
            case STATIC:
                code_instr[curCode++] = create_instr(ADDIU, val_to_reg(&t), $0, val->value.integer);
                break;
            case MEM:
                code_instr[curCode++] = create_instr(LA_, val_to_reg(&t), (int)(char*)val->value.pointer, 0);
                if(cur_type >= LINT && cur_type <= LFLOAT)
                    code_instr[curCode++] = create_instr(LW, val_to_reg(&t)
                                                           , val_to_reg(&t), 0);
                break;
            case OTHER:
            case ARG:
                code_instr[curCode++] = create_instr(MOVE, val_to_reg(&t), val_to_reg(val), 0);
                break;
            case STACK:
                code_instr[curCode++] = create_instr(LW, val_to_reg(&t), $sp, (val_sp - val->value.integer) * 4);
                break;
            default:
                break;
            }
            val->emplacement = TEMP;
            val->value.integer = i;
            temp_regs[i] = val;
            return;
        }
    nxt %= 10;
    temp_regs[i = nxt++]->emplacement = GARBAGE;
    goto cycle;
}

ValueEntry* pop()
{
    ValueEntry* ret;
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
            if(propagate_constants
                    && mips_identref[op_stack[op_sp].value.integer]
                    && mips_identref[op_stack[op_sp].value.integer]->emplacement != GARBAGE)
                ret = mips_identref[op_stack[op_sp].value.integer];
            else
            {
                ret = copy_value_entry(declarations[op_stack[op_sp].value.integer]);
                mips_identref[op_stack[op_sp].value.integer] = ret;
            }
            cur_type = type_from_identref(op_stack[op_sp].value.integer);
            if(ret->emplacement != STATIC)
                load_value(ret);
            break;
        case TIdent:
            ret = &all_values[all_values_sp++];
            ret->emplacement = IDENT_;
            ret->value = op_stack[op_sp].value;
            break;
        case TSliceident:
        {
            int identref = op_stack[op_sp].value.integer;
            ret = &all_values[all_values_sp++];
            ret->emplacement = GARBAGE;
            load_value(ret);
            if(declarations[identref]->emplacement == MEM)
                code_instr[curCode++] = create_instr(LA_, val_to_reg(ret), name_from_identref(identref), 0);
            else
            {
                code_instr[curCode++] = create_instr(MOVE, val_to_reg(ret), $sp, 0);
                code_instr[curCode++] = create_instr(ADDIU, val_to_reg(ret), val_to_reg(ret)
                                                     , (val_sp - declarations[identref]->value.integer) * 4);
            }
        }
            break;
        case TAddrtoval:
        {
            ValueEntry *tmp;
            tmp = pop();
            ret = pop();
            load_value(tmp);
            load_value(ret);
            code_instr[curCode++] = create_instr(SLL, val_to_reg(tmp), val_to_reg(tmp), 2);
            code_instr[curCode++] = create_instr(NEGU, val_to_reg(tmp), val_to_reg(tmp), 0);
            code_instr[curCode++] = create_instr(ADDU, val_to_reg(ret), val_to_reg(ret), val_to_reg(tmp));
            code_instr[curCode++] = create_instr(LW, val_to_reg(ret), val_to_reg(ret), 0);
        }
            break;
        case TCall1:
        case TCall2:
            if(c == TCall1)
                if(mips_identref[op_stack[op_sp].value.integer]
                        && mips_identref[op_stack[op_sp].value.integer]->emplacement != GARBAGE
                        && mips_identref[op_stack[op_sp].value.integer]->emplacement != STACK
                        && mips_identref[op_stack[op_sp].value.integer]->emplacement != MEM)
                    code_instr[curCode++] = create_instr(JALR, val_to_reg(mips_identref[op_stack[op_sp].value.integer]), 0, 0);
                else
                {
                    ValueEntry *t = copy_value_entry(declarations[op_stack[op_sp].value.integer]);
                    load_value(t);
                    code_instr[curCode++] = create_instr(JALR, val_to_reg(t), 0, 0);
                }
            else
                code_instr[curCode++] = create_instr(JAL, (int)(char*)op_stack[op_sp].value.pointer, 0, 0);
            had_func_call = 1;
            drop_temp_regs();
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
        else if ((c >= LREM && c <= LDIV) ||
            (c >= EQEQR && c <= LDIVR))
            prev = process_binop(c);
        else
            switch (c)
            {
            case TReturnval:
                if(prev->emplacement == STATIC)
                    code_instr[curCode++] = create_instr(ADDIU, $v0, $0, prev->value.integer);
                else
                    code_instr[curCode++] = create_instr(MOVE, $v0, val_to_reg(prev), 0);
            case TReturn:
                //  Возврат адреса окна функции
                code_instr[fp_codes[fp_codes_ptr++] = curCode++] = create_instr(LW, $fp, $sp, val_sp * 4);
                //  Возврат адреса возврата
                code_instr[curCode++] = create_instr(LW, $ra, $sp, (val_sp - 1) * 4);

                /* сброс стека */
                code_instr[curCode++] = create_instr(ADDIU, $sp, $sp, val_sp * 4);

                /* возврат по регистру $ra */
                code_instr[curCode++] = create_instr(JR, $ra, 0, 0);
                break;
            case TContinue:
                code_instr[curCode++] = create_instr(J, -1, continue_label, 0);
                break;
            case TBreak:
                code_instr[curCode++] = create_instr(J, -1, break_label, 0);
                break;
            case TPrint:
                switch (op_stack[op_sp].value.integer)
                {
                case LINT:
                    if(prev->emplacement != STATIC)
                    {
                        load_value(prev);
                        code_instr[curCode++] = create_instr(MOVE, $a0, val_to_reg(prev), 0);
                    }
                    else
                        code_instr[curCode++] = create_instr(ADDIU, $a0, $0, prev->value.integer);
                    code_instr[curCode++] = create_instr(LI_, $v0, 1, 0);
                    code_instr[curCode++] = create_instr(SYSCALL, 0, 0, 0);
                    break;
                case LFLOAT:
                    break;
                case LCHAR:
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
    if(to_ass->emplacement == MEM)
    {
        ValueEntry *c = &all_values[all_values_sp++];
        c->emplacement = GARBAGE;
        load_value(c);
        code_instr[curCode++] = create_instr(LA_, val_to_reg(c), (char*)to_ass->value.pointer, 0);
        load_value(new_val);
        code_instr[curCode++] = create_instr(SW, val_to_reg(new_val), val_to_reg(c), 0);
    }
    else if(to_ass->emplacement == STACK)
    {
        load_value(new_val);
        code_instr[curCode++] = create_instr(SW, val_to_reg(new_val), $sp, (val_sp - to_ass->value.integer) * 4);
    }
    else if(to_ass->emplacement == ARG)
    {
        load_value(new_val);
        if(new_val->emplacement == STATIC)
            code_instr[curCode++] = create_instr(ADDIU, val_to_reg(to_ass), $0, new_val->value.integer);
        else
            code_instr[curCode++] = create_instr(ADDIU, val_to_reg(to_ass), val_to_reg(new_val), 0);
    }
}

ValueEntry *process_assign(int code)
{
    if ((code >= ASS && code <= DIVASS) ||
        (code >= PLUSASSR && code <= DIVASSR) ||
        (code >= ASSV && code <= DIVASSV) ||
        (code >= PLUSASSRV && code <= DIVASSRV))
    {
        ValueEntry *a = pop(), *b = pop(), *t, *mips_ident = NULL;
        if(a->emplacement == STATIC && b->emplacement == IDENT_)
        {
            if(track_changes)
                changed_ids[curChangedId++] = create_ident_diff(b->value.integer, *mips_identref[b->value.integer]);
            if(mips_identref[b->value.integer]->emplacement == STATIC)
            {
                mips_identref[b->value.integer]->value = ret_val_int(mips_identref[b->value.integer]->value, a->value, code);
                a->emplacement = GARBAGE;
                b->emplacement = GARBAGE;
                return mips_identref[b->value.integer];
            }
            mips_ident = mips_identref[b->value.integer];
            b->emplacement = GARBAGE;
            b = declarations[b->value.integer];
        }
        else if (b->emplacement == IDENT_)
        {
            if(track_changes)
                changed_ids[curChangedId++] = create_ident_diff(b->value.integer, *mips_identref[b->value.integer]);
            mips_ident = mips_identref[b->value.integer];
            b->emplacement = GARBAGE;
            b = declarations[b->value.integer];
            b->flags &= ALL ^ CONSTANT;
        }
        //  TODO а->emplacement == STATIC
        if(code != ASS && code != ASSV)
        {
            t = copy_value_entry(b);
            if(mips_ident->emplacement == GARBAGE)
                load_value(t);
            else
                load_value(t = mips_ident);
            switch (code)
            {
            case REMASS:
            case REMASSV:
                if(a->emplacement == STATIC)
                    code_instr[curCode++] = create_instr(REMI, val_to_reg(t), val_to_reg(t), a->value.integer);
                else
                    code_instr[curCode++] = create_instr(REM, val_to_reg(t), val_to_reg(t), val_to_reg(a));
                break;
            case SHLASS:
            case SHLASSV:
                if(a->emplacement == STATIC)
                    code_instr[curCode++] = create_instr(SLL, val_to_reg(t), val_to_reg(t), a->value.integer);
                else
                    code_instr[curCode++] = create_instr(SLLV, val_to_reg(t), val_to_reg(t), val_to_reg(a));
                break;
            case SHRASS:
            case SHRASSV:
                if(a->emplacement == STATIC)
                    code_instr[curCode++] = create_instr(SRA, val_to_reg(t), val_to_reg(t), a->value.integer);
                else
                    code_instr[curCode++] = create_instr(SRAV, val_to_reg(t), val_to_reg(t), val_to_reg(a));
                break;
            case ANDASS:
            case ANDASSV:
                if(a->emplacement == STATIC)
                    code_instr[curCode++] = create_instr(ANDI, val_to_reg(t), val_to_reg(t), a->value.integer);
                else
                code_instr[curCode++] = create_instr(AND, val_to_reg(t), val_to_reg(t), val_to_reg(a));
                break;
            case EXORASS:
            case EXORASSV:
                if(a->emplacement == STATIC)
                    code_instr[curCode++] = create_instr(XORI, val_to_reg(t), val_to_reg(t), a->value.integer);
                else
                    code_instr[curCode++] = create_instr(XOR, val_to_reg(t), val_to_reg(t), val_to_reg(a));
                break;
            case ORASS:
            case ORASSV:
                if(a->emplacement == STATIC)
                    code_instr[curCode++] = create_instr(ORI, val_to_reg(t), val_to_reg(t), a->value.integer);
                else
                    code_instr[curCode++] = create_instr(OR, val_to_reg(t), val_to_reg(t), val_to_reg(a));
                break;
            case PLUSASS:
            case PLUSASSV:
                if(a->emplacement == STATIC)
                    code_instr[curCode++] = create_instr(ADDIU, val_to_reg(t), val_to_reg(t), a->value.integer);
                else
                    code_instr[curCode++] = create_instr(ADDU, val_to_reg(t), val_to_reg(t), val_to_reg(a));
                break;
            case MINUSASS:
            case MINUSASSV:
                if(a->emplacement == STATIC)
                    code_instr[curCode++] = create_instr(SUBIU, val_to_reg(t), val_to_reg(t), a->value.integer);
                else
                    code_instr[curCode++] = create_instr(SUBU, val_to_reg(t), val_to_reg(t), val_to_reg(a));
                break;
            case MULTASS:
            case MULTASSV:
                if(a->emplacement == STATIC)
                    code_instr[curCode++] = create_instr(MULI, val_to_reg(t), val_to_reg(t), a->value.integer);
                else
                    code_instr[curCode++] = create_instr(MUL, val_to_reg(t), val_to_reg(t), val_to_reg(a));
                break;
            case DIVASS:
            case DIVASSV:
                if(a->emplacement == STATIC)
                    code_instr[curCode++] = create_instr(DIVI, val_to_reg(t), val_to_reg(t), a->value.integer);
                else
                    code_instr[curCode++] = create_instr(DIV, val_to_reg(t), val_to_reg(t), val_to_reg(a));
                break;
            default:
                break;
            }
            a->emplacement = GARBAGE;
            a = t;
        }
        assign_to_ValueEntry(b, a);
        if(mips_ident)
        {
            mips_ident->emplacement = a->emplacement;
            mips_ident->value = a->value;
        }
        return a;
    }
    else if((code >= POSTINC && code <= DEC) ||
        (code >= POSTINCV && code <= DECV) ||
        (code >= POSTINCR && code <= DECR) ||
        (code >= POSTINCRV && code <= DECRV))
    {
        ValueEntry *a = pop(), *b, *ret;
        if(a->emplacement == IDENT_)
        {
            b = mips_identref[a->value.integer];
            a->emplacement = GARBAGE;
            a = declarations[a->value.integer];
        }
        if(!propagate_constants)
        {
            b->emplacement = GARBAGE;
            b = copy_value_entry(a);
        }
        if(b->emplacement != STATIC)
            load_value(b);
        ret = b;
        switch (code)
        {
        case INCV:
        case POSTINCV:
        case INC:
        case POSTINC:
            if(code == POSTINC)
            {
                ValueEntry *c = &all_values[all_values_sp++];
                c->emplacement = GARBAGE;
                load_value(c);
                if(b->emplacement == STATIC)
                    code_instr[curCode++] = create_instr(ADDIU, val_to_reg(c), $0, b->value.integer);
                else
                    //  TODO убрать c, она здесь лишняя
                    code_instr[curCode++] = create_instr(ADDIU, val_to_reg(c), val_to_reg(b), 0);
                ret = c;
            }
            if(b->emplacement == STATIC)
                b->value.integer++;
            else
                code_instr[curCode++] = create_instr(ADDIU, val_to_reg(b), val_to_reg(b), 1);
            assign_to_ValueEntry(a, b);
            break;
        case DECV:
        case POSTDECV:
        case DEC:
        case POSTDEC:
            if(code == POSTDEC)
            {
                ValueEntry *c = &all_values[all_values_sp++];
                c->emplacement = GARBAGE;
                load_value(c);
                code_instr[curCode++] = create_instr(ADDIU, val_to_reg(c), val_to_reg(b), 0);
                ret = c;
            }
            code_instr[curCode++] = create_instr(ADDIU, val_to_reg(b), val_to_reg(b), -1);
            assign_to_ValueEntry(a, b);
            break;
        default:
            break;
        }
        return ret;
    }
}

ValueEntry *process_assign_at(int code)
{
    ValueEntry *a, *b, *ret;
    a = pop();
    b = pop();
    load_value(a);
    load_value(b);
    code_instr[curCode++] = create_instr(ADDU, val_to_reg(a), val_to_reg(a), val_to_reg(b));
    b = pop();
    load_value(b);
    switch (code)
    {
    case ASSAT:
    case ASSATV:
        code_instr[curCode++] = create_instr(SW, val_to_reg(a), val_to_reg(b), 0);
        ret = b;
        break;
    default:
        break;
    }
    return ret;
}

ValueEntry *process_binop(int code)
{
    ValueEntry *a, *b, *ret;
    a = pop();
    b = pop();
    if(a->emplacement == STATIC && b->emplacement == STATIC)
    {
        a->value = ret_val_int(a->value, b->value, code);
        goto end;
    }
    load_value(a);
    load_value(b);
    switch (code)
    {
    case LOGAND:
        break;
    case LOGOR:
        break;
    case LSHL:
        code_instr[curCode++] = create_instr(SLLV, val_to_reg(a), val_to_reg(b), val_to_reg(a));
        break;
    case LSHR:
        code_instr[curCode++] = create_instr(SRAV, val_to_reg(a), val_to_reg(b), val_to_reg(a));
        break;
    case LAND:
        code_instr[curCode++] = create_instr(AND, val_to_reg(a), val_to_reg(b), val_to_reg(a));
        break;
    case LEXOR:
        code_instr[curCode++] = create_instr(XOR, val_to_reg(a), val_to_reg(b), val_to_reg(a));
        break;
    case LOR:
        code_instr[curCode++] = create_instr(OR, val_to_reg(a), val_to_reg(b), val_to_reg(a));
        break;
    case LPLUS:
        code_instr[curCode++] = create_instr(ADDU, val_to_reg(a), val_to_reg(b), val_to_reg(a));
        break;
    case LMINUS:
        code_instr[curCode++] = create_instr(SUBU, val_to_reg(a), val_to_reg(b), val_to_reg(a));
        break;
    case LMULT:
        code_instr[curCode++] = create_instr(MUL, val_to_reg(a), val_to_reg(b), val_to_reg(a));
        break;
    case LDIV:
        code_instr[curCode++] = create_instr(DIV, val_to_reg(a), val_to_reg(b), val_to_reg(a));
        break;
    case LREM:
        code_instr[curCode++] = create_instr(REM, val_to_reg(a), val_to_reg(b), val_to_reg(a));
        break;
    case EQEQ:
        code_instr[curCode++] = create_instr(SEQ, val_to_reg(a), val_to_reg(b), val_to_reg(a));
        break;
    case NOTEQ:
        code_instr[curCode++] = create_instr(SNE, val_to_reg(a), val_to_reg(b), val_to_reg(a));
        break;
    case LLT:
        code_instr[curCode++] = create_instr(SLT, val_to_reg(a), val_to_reg(b), val_to_reg(a));
        break;
    case LGT:
        code_instr[curCode++] = create_instr(SGT, val_to_reg(a), val_to_reg(b), val_to_reg(a));
        break;
    case LLE:
        code_instr[curCode++] = create_instr(SLE, val_to_reg(a), val_to_reg(b), val_to_reg(a));
        break;
    case LGE:
        code_instr[curCode++] = create_instr(SGE, val_to_reg(a), val_to_reg(b), val_to_reg(a));
        break;
    }
end:
    b->emplacement = GARBAGE;
    ret = a;
    return ret;
}

void process_for()
{
    int cond_label = curTempLabel++, _break_label = break_label, _continue_label = continue_label;
    int old_ct, body_ct;
    ValueEntry *tmp;
    break_label = curTempLabel++, continue_label = curTempLabel++;
    body_ct = tree[curTree + 3];
    curTree += 4;
    //  инициализация
    process_expression();
    eval_dynamic();
    //  проверка условия
    propagate_constants = 0;
    code_instr[curCode++] = create_instr(LABEL, -1, cond_label, 0);
    drop_temp_regs();
    process_expression();
    tmp = eval_dynamic();
    load_value(tmp);
    code_instr[curCode++] = create_instr(BEQZ, val_to_reg(tmp), -1, break_label);
    //  Тело цикла
    old_ct = curTree;
    curTree = body_ct;
    process_block();
    //  инкремент
    old_ct ^= curTree;
    curTree ^= old_ct;
    old_ct ^= curTree;
    code_instr[curCode++] = create_instr(LABEL, -1, continue_label, 0);
    process_expression();
    eval_dynamic();
    code_instr[curCode++] = create_instr(J, -1, cond_label, 0);

    //  Выход из цикла
    curTree = old_ct;
    code_instr[curCode++] = create_instr(LABEL, -1, break_label, 0);
    propagate_constants = 1;
    break_label = _break_label, continue_label = _continue_label;
}

void process_if()
{
    int else_ref = tree[++curTree];
    int else_label = curTempLabel++, exit_label = curTempLabel++;
    ValueEntry *cond;
    //  условие
    process_expression();
    cond = eval_dynamic();

    /* если условие постоянно */
    if(cond->emplacement == STATIC)
        if(cond->value.integer)
            process_block();
        else if(else_ref)
        {
            curTree = else_ref;
            process_block();
        }
        else
            skip_block();
    else
    {
        load_value(cond);
        code_instr[curCode++] = create_instr(BEQZ, val_to_reg(cond), -1, else_ref ? else_label : exit_label);
        //  если условие выполнено
        process_block();
        //  иначе
        if(else_ref)
        {
            code_instr[curCode++] = create_instr(LABEL, -1, else_label, 0);
            process_block();
        }
        code_instr[curCode++] = create_instr(LABEL, -1, exit_label, 0);
    }
}

void process_while()
{
    int _break_label = break_label, _continue_label = continue_label;
    ValueEntry *tmp;
    break_label = curTempLabel++, continue_label = curTempLabel++;

    propagate_constants = 0;
    drop_temp_regs();

    /* условие */
    code_instr[curCode++] = create_instr(LABEL, -1, continue_label, 0);
    process_expression();
    tmp = eval_dynamic();
    load_value(tmp);
    code_instr[curCode++] = create_instr(BEQZ, val_to_reg(tmp), -1, break_label);

    /* тело */
    process_block();
    code_instr[curCode++] = create_instr(LABEL, -1, break_label, 0);

    break_label = _break_label, continue_label = _continue_label;
    propagate_constants = 1;
}

void process_do_while()
{
    int _break_label = break_label, _continue_label = continue_label, body_label = curTempLabel++;
    ValueEntry *tmp;
    break_label = curTempLabel++, continue_label = curTempLabel++;

    propagate_constants = 0;
    drop_temp_regs();

    /* тело */
    code_instr[curCode++] = create_instr(LABEL, -1, body_label, 0);
    process_block();

    /* условие */
    code_instr[curCode++] = create_instr(LABEL, -1, continue_label, 0);
    process_expression();
    tmp = eval_dynamic();
    load_value(tmp);
    code_instr[curCode++] = create_instr(BNEZ, val_to_reg(tmp), -1, body_label);
    code_instr[curCode++] = create_instr(LABEL, -1, break_label, 0);

    break_label = _break_label, continue_label = _continue_label;
    propagate_constants = 1;
}

void process_switch()
{
    ValueEntry *var;
    int _break = break_label;
    break_label = curTempLabel++;
    /* выражение для перечисления */
    process_expression();
    var = eval_dynamic();
    curTree++;
    start_track_changes();

    /* TODO исправить переход в случае "проваливающегося" case, т.е.:
        case 5:
        ...
        // нет break;
        case 15:
        ...
        break;  */
    while(tree[curTree] != TEnd)
    {
        ValueEntry *res;
        int old_code = curCode;
        int is_def = 0;
        curChangedId = 0;
        while(tree[curTree] != TCase && tree[curTree] != TDefault
              && tree[curTree] != TEnd)
        {
            process_block();
            res = eval_dynamic();
        }
        /* TODO восстанавливать ТОЛЬКО после break */
        restore_from_diff();
        if(tree[curTree] == TDefault)
            is_def = 1;
        if(var->emplacement == STATIC)
        {
            if(!is_def || res->value.integer != var->value.integer)
                curCode = old_code;
            continue;
        }
        else
        {
            code_instr[curCode++] = create_instr(LABEL, -1, curTempLabel++, 0);
            load_value(var);
            if(res->emplacement == STATIC)
            {
                int r = res->value.integer;
                res->emplacement = GARBAGE;
                load_value(res);
                code_instr[curCode++] = create_instr(SEQI, val_to_reg(res), val_to_reg(var), r);
            }
            else
            {
                load_value(var);
                code_instr[curCode++] = create_instr(SEQ, val_to_reg(res), val_to_reg(var), val_to_reg(res));
            }
            code_instr[curCode++] = create_instr(BEQZ, val_to_reg(res), -1, curTempLabel);
        }
        if(tree[curTree] != TEnd)
            curTree += 2;
        else
            break;
    }
    stop_track_changes();
    code_instr[curCode++] = create_instr(LABEL, -1, curTempLabel++, 0);
    drop_temp_regs();
    code_instr[curCode++] = create_instr(LABEL, -1, break_label, 0);
    break_label = _break;
    curTree++;
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
                IdentEntry data;
                ValueEntry *val = &all_values[all_values_sp++];
                if(initref)
                {
                    process_expression();
                    data.value = eval_static();
                }
                data.name = name_from_identref(identref);
                data.type = type_from_identref(identref);
                if(level)
                {
                    if (initref)
                    {
                        code_instr[curCode++] = create_instr(ADDIU, $t0, $0, data.value.integer);
                        code_instr[curCode++] = create_instr(SW, $t0, $sp, (val_sp - old_val_sp + 1) * 4);
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
                declarations[identref] = &all_values[all_values_sp++];
                assign_ValueEntry(declarations[identref], val);
                if(level)
                {
                    val->emplacement = STATIC;
                    if(initref)
                        val->value = data.value;
                    else
                        val->value.integer = 0;
                }
                mips_identref[identref] = val;
                break;
            }
            case 1:
            case 2:
            {
                IdentEntry data;
                Value temp;
                ValueEntry *val = &all_values[all_values_sp++];
                int size;
                data.value.pointer = NULL;
                process_expression();
                temp = eval_static();
                size = temp.integer;
                if(N == 2)
                {
                    process_expression();
                    temp = eval_static();
                    size *= temp.integer;
                }
                if(initref)
                {
                    int i;
                    Value *buffer;
                    buffer = malloc(size * sizeof(Value));
                    for(i = 0; i < size; ++i)
                    {
                        process_expression();
                        temp = eval_static();
                        buffer[i] = temp;
                    }
                    data.value.pointer = buffer;
                    if(level)
                    {
                        for(i = 0; i < size; ++i)
                        {
                            code_instr[curCode++] = create_instr(ADDIU, $t0, $0, buffer[i].integer);
                            code_instr[curCode++] = create_instr(SW, $t0, $sp, (val_sp + i - old_val_sp + 1) * 4);
                        }
                    }
                }
                data.size = size;
                data.name = name_from_identref(identref);
                data.type = type_from_identref(identref);
                if(level)
                {
                    val->emplacement = STACK;
                    val->value.integer = val_sp;
                    val_sp += size;
                }
                else
                {
                    val->emplacement = MEM;
                    val->value.pointer = data.name;
                    data_entries[curData++] = data;
                }
                mips_identref[identref] = val;
                declarations[identref] = &all_values[all_values_sp++];
                assign_ValueEntry(&declarations[identref], val);
            }
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
            drop_temp_regs();
            level = 0;
            break;
        }
        default:
            break;
    }
}

void process_function()
{
    int identref = tree[curTree++], args, sp_add_code;
    int i;
    ValueEntry *func = &all_values[all_values_sp++];
    curTree++;
    code_instr[curCode++] = create_instr(LABEL, name_from_identref(identref), 0, 0);

    /* заполнить определение */
    func->emplacement = MEM;
    func->value.pointer = name_from_identref(identref);
    declarations[identref] = func;

    /* размер стэка */
    val_sp = 0;

    /* Аргументы функции */
    args = modetab[identab[identref + 2] + 1];
    for(i = 0; i < (args > 4 ? 4 : args); ++i)
    {
        ValueEntry *val = &all_values[all_values_sp++];
        val->emplacement = ARG;
        val->value.integer = i;
        mips_identref[tree[curTree]] = declarations[tree[curTree]] = val;
        ++curTree;
    }
    for(i = 0; i < args - 4; ++i)
    {
        ValueEntry *val = &all_values[all_values_sp++];
        val->emplacement = STACK;
        val->value.integer = val_sp++;
        mips_identref[tree[curTree]] = declarations[tree[curTree]] = val;
        ++curTree;
    }
    curTree++;
    val_sp += 2;

    /* Сохранение указателя на окно функции и адреса возврата */
    code_instr[sp_add_code = curCode++] = create_instr(ADDIU, $sp, $sp, -val_sp * 4);
    code_instr[curCode++] = create_instr(SW, $fp, $sp, val_sp * 4);
    code_instr[curCode++] = create_instr(SW, $ra, $sp, (val_sp - 1) * 4);

    /* Запись адреса нового окна функции */
    code_instr[curCode++] = create_instr(MOVE, $fp, $sp, 0);

    fp_codes_ptr = had_func_call = 0;

    /* Определения */
    while (tree[curTree] == TDeclid)
        process_declaration(val_sp);

    code_instr[sp_add_code + 0].third_op = -val_sp * 4;
    code_instr[sp_add_code + 1].third_op = val_sp * 4;
    code_instr[sp_add_code + 2].third_op = (val_sp - 1) * 4;

    /* Выражения */
    while (tree[curTree] != TEnd)
    {
        process_expression();
        eval_dynamic();
    }
    if(!had_func_call)
    {
        int i = 0;
        val_sp -= 2;
        code_instr[sp_add_code + 1].code = code_instr[sp_add_code + 2].code = code_instr[sp_add_code + 3].code = DELETED;
        for(i; i < fp_codes_ptr; ++i)
        {
            code_instr[fp_codes[i]].code = code_instr[fp_codes[i] + 1].code = DELETED;
            if(val_sp)
            {
                code_instr[sp_add_code].third_op = -val_sp * 4;
                code_instr[fp_codes[i] + 2].third_op = val_sp * 4;
            }
            else
            {
                code_instr[sp_add_code].code = DELETED;
                code_instr[fp_codes[i] + 2].code = DELETED;
            }
        }
    }
    val_sp = 0;
}

void process_expression()
{
    while (tree[curTree] != TExprend && tree[curTree] != TEnd)
    {
        Operation oper;
        int c, i, size;
        switch (c = tree[curTree++])
        {
        case TConst:
        case TIdent:
        case TIdenttoval:
            oper.code = c;
            oper.value.integer = tree[curTree++];
            op_stack[op_sp++] = oper;
            break;
        case TSliceident:
            if(op_sp && op_stack[op_sp - 1].code == TIdent)
                --op_sp;
            oper.code = TSliceident;
            oper.value.integer = tree[curTree++];
            op_stack[op_sp++] = oper;
            process_expression();
            break;
        case TFor:
            process_for();
            break;
        case TIf:
            process_if();
            break;
        case TSwitch:
            process_switch();
            break;
        case TWhile:
            curTree++;
            process_while();
            break;
        case TDo:
            curTree++;
            process_do_while();
            break;
        case TLabel:
        case TGoto:
            curTree++;
            break;
        case TCall1:
            if((size = tree[curTree++]) > 4);
            for(i = 0; i < size; ++i)
            {
                ValueEntry *r;
                process_expression();
                r = pop();
                if(r->emplacement == STATIC)
                    code_instr[curCode++] = create_instr(ADDIU, $a0 + i, $0, r->value.integer);
                else
                    code_instr[curCode++] = create_instr(ADDIU, $a0 + i, val_to_reg(r), 0);
            }
            break;
        case TCall2:
            if(declarations[-tree[curTree]]->emplacement == MEM)
            {
                oper.code = TCall2;
                oper.value.pointer = name_from_identref(-tree[curTree++]);
            }
            else
            {
                oper.code = TCall1;
                oper.value.integer = -tree[curTree++];
            }
            --op_sp;
            op_stack[op_sp++] = oper;
            break;
        case TPrint:
            oper.code = TPrint;
            oper.value.integer = tree[curTree++];
            op_stack[op_sp++] = oper;
            return;
        case TReturn:
            oper.code = TReturn;
            op_stack[op_sp++] = oper;
            return;
        case TReturnval:
        case TAddrtoval:
        case TBreak:
        case TContinue:
            oper.code = c;
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
    if(tree[curTree] == TBegin)
    {
        ++curTree;
        while (tree[curTree] == TDeclid)
            process_declaration(old_val_sp);
        //  рост стека
        if(val_sp != old_val_sp)
            code_instr[curCode++] = create_instr(ADDIU, $sp, $sp, -(val_sp - old_val_sp) * 4);
        while (tree[curTree] != TEnd)
        {
            process_expression();
            eval_dynamic();
        }
        curTree++;
    }
    else
    {
        process_expression();
        eval_dynamic();
    }
    if(val_sp != old_val_sp)
    {
        code_instr[curCode++] = create_instr(ADDIU, $sp, $sp, (val_sp - old_val_sp) * 4);
        val_sp = old_val_sp;
    }
}

void skip_block()
{
    if(tree[curTree] == TBegin)
        while (tree[curTree++] != TEnd);
    else
        while (tree[curTree] != TExprend && tree[curTree] != TEnd)
            ++curTree;
    if(tree[curTree] == TExprend)
        ++curTree;
}

void mark_block()
{
    int _tree = curTree;
    if(tree[_tree] == TBegin)
        while (tree[_tree++] != TEnd);
    else
        while (tree[_tree] != TExprend && tree[_tree] != TEnd)
            ++_tree;
    if(tree[_tree] == TExprend)
        ++_tree;
}

void compile_mips()
{
    int i;
    out_mips = fopen(outfile, "w");
    curTree = curData = curInit = curCode = curChangedId = 0;
    curTempLabel = 0;
    op_sp = val_sp = all_values_sp = 0;
    track_changes = 0;
    propagate_constants = 1;
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
    write_instr(create_instr(J, name_from_identref(0), 0, 0));
    for(i = 0; i < curCode; i++)
        write_instr(code_instr[i]);

    fclose(output);
    return;
}
