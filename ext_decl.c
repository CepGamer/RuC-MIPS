
#include "global_vars.h"

extern int  getnext();
extern int  nextch();
extern int  scaner();
extern void error(int e);

void mustbe(int what, int e)
{
    if (scaner() != what)
        error(e);
}

void totree(int op)
{
    tree[curTree++] = op;
}

void totreef(int op)
{
    tree[curTree++] = op;
    if ( ansttype == LFLOAT && ((op >= PLUSASS && op <= DIVASS) || (op >= PLUSASSAT && op <= DIVASSAT) ||
                                (op >= EQEQ && op <= UNMINUS)) )
        tree[curTree - 1] += 50;
}

int toidentab(int f)                 // f=0, если не ф-ция, f=1, если метка, f=funcnum, если описание ф-ции и -1, если ф-ция-параметр
{
//    printf("\n repr %i rtab[repr] %i rtab[repr+1] %i rtab[repr+2] %i\n", repr, reprtab[repr], reprtab[repr+1], reprtab[repr+2]);
    int pred;
    lastid = id;
    if (reprtab[repr+1] == 0)                  // это может быть только MAIN
    {
        if (wasmain)
            error(more_than_1_main);
        wasmain = id;
    }
    pred = identab[id] = reprtab[repr+1];      // ссылка на описание с таким же представлением в предыдущем блоке
    if (pred)                                  // pred == 0 только для main, эту ссылку портить нельзя
        reprtab[repr+1] = id;                  // ссылка на текущее описание с этим представлением (это в reprtab)
    id++;
    
    if (f != 1 && pred >= curid)               // один  и тот же идент м.б. переменной и меткой
        if (func_def == 3 ? 1 : identab[pred+1] > 0 ? 1 : func_def ==  1 ? 0 : 1)
            error(repeated_decl);              // только определение функции может иметь 2 описания, т.е. иметь предописание
    
    identab[id++] = repr;                      // ссылка на представление
                                               // дальше тип или ссылка на modetab (для функций и структур)
    identab[id++] = type;                      // тип -1 int, -2 char, -3 float, -5 []int, -6 []char, -7 []float,
                                               //                       -15 [][]int, -16 [][]char, -17 [][]float
                                               //-9 func int, -10 funcchar, -11 func float, -12 func void, 0-метка
                                               // если на 20 меньше, то это указатель на нужный тип
    if (f == 1)                                // если тип > 0, то это ссылка на modetab (для деклараторов и структур)
    {
        identab[id-1] = 0;                     // 0, если первым встретился goto, когда встретим метку, поставим 1
        identab[id++] = 0;                     // при генерации кода когда встретим метку, поставим pc
    }
    else if (f)
    {
            if (f < 0)
            {
                identab[id] = -(++displ);
                maxdispl = (displ > maxdispl) ? displ : maxdispl;
            }
            else                                                   // identtab[lastid+3] - номер функции, если < 0, то это функция-параметр
            {
                identab[id] = f;
                if (func_def == 2)
                    identab[lastid+1] *= -1;
            }
//        printf("toident id=%i lastid=%i func_def=%i identab[id]=%i\n", id, lastid, func_def, identab[id]);
        id++;
    }
    else
    {
        identab[id++] = displ += lg;            // смещение от l (полож) или от g (отриц), для меток - значение pc, для функций - номер
        
        if (lg < 0)
            maxdisplg = -displ;
        else
            maxdispl = (displ > maxdispl) ? displ : maxdispl;
    }
    return lastid;
}

void binop(int op)
{
    int rtype = stackoperands[sopnd--];
    int ltype = stackoperands[sopnd];
    if ((ltype == LINT || ltype == LCHAR) && rtype == LFLOAT)
        totree(WIDEN1);
    if ((rtype == LINT || rtype == LCHAR) && ltype == LFLOAT)
        totree(WIDEN);
    if (ltype == LFLOAT || rtype == LFLOAT)
        ansttype = LFLOAT;
    totreef(op);
    if (op >= EQEQ && op <= LGE)
        ansttype = LINT;
    stackoperands[sopnd] = ansttype;
//    printf("binop sopnd=%i ltype=%i rtype=%i ansttype=%i\n", sopnd, ltype, rtype, ansttype);
    anst = VAL;
}

void expr();

void exprassn(int);

void toval()
{
    if (anst == IDENT)
        tree[curTree - 2] = TIdenttoval;
    if (ansttype != ROWOFCHAR)
    {
        if (anst == ADDR)
            totree(TAddrtoval);
        anst = VAL;
    }
}

void insertwiden()
{
    curTree--;
    totree(WIDEN);
    totree(TExprend);
}

void primaryexpr()
{
    if (cur == NUMBER)
    {
        totree(TConst);                              // ansttype задается прямо в сканере
        totree((ansttype == LFLOAT) ? numr : num);   // LINT, LCHAR, FLOAT
        stackoperands[++sopnd] = ansttype;
//        printf("number sopnd=%i ansttype=%i\n", sopnd, ansttype);
        anst = NUMBER;
    }
    else if (cur == STRING)
    {
        int i = 0;
        totree(TString);
        do
            totree(lexstr[i]);
        while (lexstr[i++]);
        stackoperands[++sopnd] = ansttype;           // ROWOFCHAR
        anst = ADDR;
    }
    else if (cur == IDENT)
    {
        lastid = reprtab[repr+1];
        if (lastid == 1)
            error(ident_is_not_declared);
        totree(TIdent);
        totree(lastid);
        stackoperands[++sopnd] = ansttype  = identab[lastid+2];
//        printf("ident sopnd=%i ansttype=%i\n", sopnd,ansttype);
        anst = IDENT;
        anstdispl = identab[lastid+3];
    }
    else if (cur == LEFTBR)
    {
        int oldsp = sp;
        scaner();
        expr();
        mustbe(RIGHTBR, wait_rightbr_in_primary);
        while (sp > oldsp)
            binop(stackop[--sp]);
    }
    else if (cur <= -23)            // стандартная функция
    {
        int func = cur;
        mustbe(LEFTBR, no_leftbr_in_stand_func);
        scaner();
        exprassn(1);
        if (func == GETDIGSENSOR || func == GETANSENSOR)
        {
            notrobot = 0;
            if (ansttype != LINT)
                error(param_setmotor_not_int);
            totree(400-func);
        }
        else if (func == ABS)
                if (ansttype == LINT)
                     totree(ABSIC);
                else if (ansttype == LFLOAT)
                         totree(ABSC);
                     else
                         error(bad_param_in_stand_func);
        else
        {
            if (ansttype == LINT)
            {
                ansttype = stackoperands[sopnd] = LFLOAT;
                totree(WIDEN);
            }
            totree(400-func);
        }
        mustbe(RIGHTBR, no_rightbr_in_stand_func);
    }
    else
        error(not_primary);
}

void postexpr()
{
    int lid, leftanst, leftansttype;
    primaryexpr();
    leftanst = anst;
    lid = lastid;
    leftansttype = ansttype;
    if (next == LEFTSQBR)
    {
        if  (ansttype != ROWOFINT && ansttype != ROWOFCHAR && ansttype != ROWOFFLOAT &&
              ansttype != ROWROWOFINT && ansttype != ROWROWOFCHAR && ansttype != ROWROWOFFLOAT)
             error(slice_not_from_array);
        scaner();
        scaner();
        if (leftanst == IDENT)
        {
            totree(TSliceident);
            totree(lid);
            expr();
        }
        else
        {
            expr();
            totree(TSlice);                       // STRING
        }
        mustbe(RIGHTSQBR, no_rightsqbr_in_slice);
        if (ansttype != LINT && ansttype != LCHAR)
            error(index_must_be_int);
        
        if (next == LEFTSQBR)
        {
            scaner();
            scaner();
            expr();
            totree(TSlice);
            mustbe(RIGHTSQBR, no_rightsqbr_in_slice);
            if (ansttype != LINT && ansttype != LCHAR)
                error(index_must_be_int);
            --sopnd;
            stackoperands[--sopnd] = ansttype = leftansttype + 14;
        }
        else
            stackoperands[--sopnd] = ansttype = leftansttype + 4;
        anst = ADDR;
     }
     else if (next == LEFTBR)
     {
         int i, j, n;
         scaner();
         if (leftansttype < 0)
              error(call_not_from_function);
         if (identab[lid+1] < 0)
             error(declarator_in_call);
         
         n = modetab[leftansttype+1];
         
         totree(TCall1);
         totree(n);
         j = leftansttype + 2;
         for (i=0; i<n; i++)
         {
             int mdj = modetab[j];  // это вид формального параметра, в ansttype будет вид фактического параметра
             scaner();
             if (mdj > 0)
             {
                 int ld = reprtab[repr+1];
                 if( ! (cur == IDENT && identab[ld+2] == mdj) )
                     error(diff_formal_param_type_and_actual);
                 totree(TIdenttoval);
                 totree(ld);
                 totree(TExprend);
             }
             else
             {
                 exprassn(0);
                 if ((mdj == LINT || mdj == LCHAR) && ansttype == LFLOAT)
                     error(float_instead_int);
                 if (mdj == LFLOAT && (ansttype == LINT || ansttype == LCHAR))
                     insertwiden();
                 if (ansttype != mdj)
                     error(diff_formal_param_type_and_actual);
                 --sopnd;
             }
             if (i < n-1 && scaner() != COMMA)
                 error(no_comma_in_act_params);
             j++;
         }
         mustbe(RIGHTBR, wrong_number_of_params);
         totree(TCall2);
         totree(-lid);
         stackoperands[sopnd] = ansttype = modetab[leftansttype] + 8;   //  --?
         anst = VAL;
     }
     else if (leftansttype > 0)
        error(func_not_in_call);
     else if (leftansttype !=LINT && leftansttype != LCHAR && leftansttype != LFLOAT && leftansttype != ROWOFCHAR)
        error(wrong_operand);
     if (next == INC || next == DEC)
     {
         if  (leftanst != IDENT && anst != ADDR)
             error(unassignable_inc);
         int op = (next== INC) ? POSTINC : POSTDEC;
         if (anst ==ADDR)
             op += 4;
         scaner();
         totreef(op);
         if (leftanst == IDENT)
             totree(-lid);
         anst = VAL;
     }
}

void unarexpr()
{
    if (cur == LNOT || cur == LOGNOT || cur == LPLUS || cur == LMINUS || cur == LAND || cur == LMULT)
    {
        int op  = cur;
        scaner();
        unarexpr();
        if ((op == LNOT || op == LOGNOT) && ansttype == LFLOAT)
            error(int_op_for_float);
        if (op == LAND)
        {
            totree(ANDADDR);
            ansttype -= 20;
        }
        else
        {
            toval();
            if (op == LMULT)
            {
                if (ansttype > -20)
                    error(aster_not_for_pointer);
                totree(MULTADDR);
                ansttype += 20;
            }
            else if (op == LMINUS)
                totreef(UNMINUS);
            else if (op == LPLUS)
                ;
            else
                totree(op);
            stackoperands[sopnd] = ansttype;
            anst = VAL;
        }
    }
    else if (cur == INC || cur == DEC)
    {
        int op = cur;
        scaner();
        postexpr();
        if  (anst != IDENT && anst != ADDR)
            error(unassignable_inc);
        if (anst == ADDR)
            op += 4;
        totreef(op);
        if (anst == IDENT)
            totree(-lastid);
        anst = VAL;
    }
    else
        postexpr();
}

void exprinbrkts()
{
    mustbe(LEFTBR, cond_must_be_in_brkts);
    scaner();
    expr();
    mustbe(RIGHTBR, cond_must_be_in_brkts);
}

int prio(int op)   // возвращает 0, если не операция
{
    return  op == LOGOR ? 1 : op == LOGAND ? 2 : op == LOR ? 3 : op == LEXOR ? 4 : op == LAND ? 5 :
    op == EQEQ ? 6 : op == NOTEQ ? 6 :
    op == LLT  ? 7 : op == LGT ?   7 : op  == LLE ? 7 : op == LGE ? 7 :
    op == LSHL ? 8 : op == LSHR ? 8 : op == LPLUS ? 9 : op == LMINUS ? 9 :
    op == LMULT ? 10 : op == LDIV ? 10 : op == LREM ? 10 : 0 ;
}

void subexpr()
{
    int p, oldsp = sp;
    while ((p = prio(next)))
    {
        while (sp > oldsp && stack[sp-1] >= p)
            binop(stackop[--sp]);
        
        stack[sp] = p;
        stackop[sp++] = next;
        scaner();
        scaner();
        unarexpr();
        toval();
    }
    while (sp > oldsp)
        binop(stackop[--sp]);
}

int opassn()
{
    op = next;
    return  next == ASS || next == MULTASS || next == DIVASS ||next == REMASS || next == PLUSASS || next == MINUSASS ||
    next == SHLASS || next == SHRASS || next == ANDASS || next == EXORASS || next == ORASS;
}

void condexpr()
{
    int globtype = LINT, adif = 0, r;
    toval();
    subexpr();                   // logORexpr();
    if (next == QUEST)
    {
        while (next == QUEST)
        {
            int thenref, elseref;
            if (ansttype == LFLOAT)
                error(float_in_condition);
            totree(TCondexpr);
            thenref = curTree++;
            elseref = curTree++;
            scaner();
            scaner();
            sopnd--;
            tree[thenref] = curTree;
            expr();                  // then
            sopnd--;
            if (ansttype == LFLOAT)
                globtype = LFLOAT;
            else
            {
                tree[curTree] = adif;
                adif = curTree++;
            }
            mustbe(COLON, no_colon_in_cond_expr);
            scaner();
            tree[elseref] = curTree;
            unarexpr();
            toval();
            subexpr();   // logORexpr();        else or elif
        }
        totree(TExprend);
        if (ansttype == LFLOAT)
            globtype = LFLOAT;
        else
        {
            tree[curTree] = adif;
            adif = curTree++;
        }
        do
        {
            r = tree[adif];
            tree[adif] = TExprend;
            tree[adif-1] = globtype == LFLOAT ? WIDEN : NOP;
            adif = r;
        }
        while (adif);
        
            stackoperands[sopnd] = globtype;
//        printf("cond1 sopnd=%i globtype=%i\n", sopnd, globtype);
    }
    else
        stackoperands[sopnd] = ansttype;
//    printf("cond2  sopnd=%i ansttype=%i\n", sopnd, ansttype);
}

void exprassnvoid()
{
    int t = tree[curTree - 2] < 0 ? curTree - 3 : curTree - 2;
    if ((tree[t] >= ASS && tree[t] <= DIVASSAT)     || (tree[t] >= POSTINC && tree[t] <= DECAT) ||
        (tree[t] >= PLUSASSR && tree[t] <= DIVASSATR) || (tree[t] >= POSTINCR && tree[t] <= DECATR))
         tree[t] += 200;
    --sopnd;
}

void exprassn(int level)
{
    int opp = 0, leftanst, lid;
    unarexpr();
    leftanst = anst;
    lid = lastid;
    while (opassn())
    {
        scaner();
        scaner();
        opp = op;
        exprassn(level + 1);
    }
    if (opp)
    {
        int rtype = stackoperands[sopnd--];
        int ltype = stackoperands[sopnd--];
        if ((ltype == LINT || ltype == LCHAR) && rtype == LFLOAT)
            error(assmnt_float_to_int);
        if ((rtype == LINT ||rtype == LCHAR) && ltype == LFLOAT)
            totree(WIDEN);
        if (ltype == LFLOAT || rtype == LFLOAT)
            ansttype = LFLOAT;
        stackoperands[sopnd] = ansttype;
        if (leftanst == ADDR)
            opp += 11;
        totreef(opp);
        if (leftanst ==IDENT)
            totree(-lid);
    }
    else
        condexpr();    // condexpr учитывает тот факт, что начало выражения в виде unarexpr уже выкушано
    if (level == 0)
        totree(TExprend);
}

void expr()
{
    exprassn(0);
    while (next == COMMA)
    {
        exprassnvoid();
        sopnd--;
        scaner();
        scaner();
        exprassn(0);
    }
}

int arrinit(int decl_type)
{
    int ni = 0;
    scaner();
    mustbe(BEGIN, arr_init_must_start_from_BEGIN);
    while (next != END)
    {
        ni++;
        scaner();
        exprassn(0);
        sopnd--;
        if ((decl_type == LINT || decl_type == LCHAR) && ansttype == LFLOAT)
            error(init_int_by_float);
        if (decl_type == LFLOAT && ansttype != LFLOAT)
            insertwiden();
        if (next == COMMA)
            scaner();
        else if (next != END)
            error(no_comma_in_init_list);
    }
    scaner();
    return ni;

}

void decl_id()
{
    int oldid = toidentab(0);
    int decl_type = type;
    int initref, n, ni;     // n - размерность (0-скаляр), д.б. столько выражений-границ, инициализатор начинается с L  
    totree(TDeclid);
    totree(oldid);
    initref = curTree++;
    tree[n = curTree++] = 0;
    if (next == ASS)
    {
        scaner();
        scaner();
        tree[initref] = curTree;
        exprassn(0);
        sopnd--;
        if ((decl_type == LINT || decl_type == LCHAR) && ansttype == LFLOAT)
            error(init_int_by_float);
        if (decl_type == LFLOAT && ansttype != LFLOAT)
            insertwiden();
    }
    else if (next == LEFTSQBR)
    {
        scaner();
        scaner();
        tree[n] = 1;
        identab[oldid+2] -= 4;
        unarexpr();
        condexpr();
        totree(TExprend);
        sopnd--;
        mustbe(RIGHTSQBR, wait_right_sq_br);
        if (next == ASS)
        {
            tree[initref] = curTree;
            totree(TInit);
            ni = curTree++;
            tree[ni] = arrinit(decl_type);
        }
        else if (next == LEFTSQBR)
        {
            scaner();
            scaner();
            tree[n]++;
            identab[oldid+2] -= 10;
            unarexpr();
            condexpr();
            totree(TExprend);
            sopnd--;
            mustbe(RIGHTSQBR, wait_right_sq_br);
            if (next == ASS)
            {
                tree[initref] = curTree;
                totree(TInit);
                ni = curTree++;
                tree[ni] = arrinit(decl_type);
            }
        }
    }
}

void block(int b);
 // если b=1, то это просто блок, b=-1 - блок в switch, иначе (b=0) - это блок функции

void statement()
{
    int flagsemicol = 1, oldwasdefault = wasdefault, oldinswitch = inswitch;
    wasdefault = 0;
    scaner();
    if ((cur == LINT || cur == LCHAR || cur == LFLOAT || cur == LVOID) && blockflag)
        error(decl_after_strmt);
    if (cur == BEGIN)
    {
        flagsemicol = 0;
        block(1);
    }
    else if (cur == SEMICOLON)
        flagsemicol = 0;
    else if (cur == IDENT && next == COLON)
    {
        int id, i, flag = 1;
        flagsemicol = 0;
        totree(TLabel);
        for (i=0; flag && i < pgotost-1; i+=2)
            flag = identab[ gotost[i]+1] != repr;
        if (flag)
        {
            totree(id = toidentab(1));
            gotost[pgotost++] = id;              // это определение метки, если она встретилась до переходов на нее
            gotost[pgotost++] = -line;
        }
        else
        {
            id = gotost[i-2];
            repr = identab[id+1];
            if (gotost[i-1] < 0)
                error(repeated_label);
            totree(id);
        }
        identab[id+2] = 1;

        scaner();
        statement();
    }
    else
    {
        blockflag = 1;
        switch (cur)
        {
            case PRINT:
            {
                exprinbrkts();
                totree(TPrint);
                totree(ansttype);
                sopnd --;
            }
                break;
            case PRINTID:
            {
                mustbe(LEFTBR, no_leftbr_in_printid);
                mustbe(IDENT, no_ident_in_printid);
                lastid = reprtab[repr+1];
                if (lastid == 1)
                    error(ident_is_not_declared);
                totree(TPrintid);
                totree(lastid);
                mustbe(RIGHTBR, no_rightbr_in_printid);
            }
                break;
        case GETID:
            {
                mustbe(LEFTBR, no_leftbr_in_printid);
                mustbe(IDENT, no_ident_in_printid);
                lastid = reprtab[repr+1];
                if (lastid == 1)
                    error(ident_is_not_declared);
                totree(TGetid);
                totree(lastid);
                mustbe(RIGHTBR, no_rightbr_in_printid);
                break;
            }
            case SETMOTOR:
            {
                notrobot = 0;
                mustbe(LEFTBR, no_leftbr_in_setmotor);
                scaner();
                exprassn(0);
                if (ansttype != LINT)
                    error(param_setmotor_not_int);
                mustbe(COMMA, no_comma_in_setmotor);
                scaner();
                exprassn(0);
                if (ansttype != LINT)
                    error(param_setmotor_not_int);
                sopnd -= 2;
                totree(SETMOTOR);
                mustbe(RIGHTBR, no_rightbr_in_setmotor);
                break;
            }
            case SLEEP:
            {
                notrobot = 0;
                mustbe(LEFTBR, no_leftbr_in_sleep);
                scaner();
                exprassn(0);
                if (ansttype != LINT)
                    error(param_setmotor_not_int);
                sopnd --;
                totree(SLEEP);
                mustbe(RIGHTBR, no_rightbr_in_sleep);
                break;
            }
            case LBREAK:
            {
                if (! (inloop || inswitch))
                    error(break_not_in_loop_or_switch);
                totree(TBreak);
            }
                break;
            case LCASE:
            {
                if (! inswitch)
                    error(case_or_default_not_in_switch);
                if (wasdefault)
                    error(case_after_default);
                scaner();
                flagsemicol = 0;
                unarexpr();
                condexpr();
                totree(TExprend);
                if (ansttype == LFLOAT)
                    error(float_in_switch);
                sopnd--;
                if (next != COLON)
                    error(no_colon_in_case);
                scaner();
                totree(TCase);
                totree(num);
                statement();
            }
                break;
            case LCONTINUE:
            {
                if (! inloop)
                    error(continue_not_in_loop);
                totree(TContinue);
            }
                break;
            case LDEFAULT:
            {
                if (! inswitch)
                    error(case_or_default_not_in_switch);
                if (next != COLON)
                    error(no_colon_in_case);
                wasdefault = 1;
                totree(TDefault);
                scaner();
                statement();
            }
                break;
            case LDO:
            {
                inloop = 1;
                int condref;
                totree(TDo);
                condref = curTree++;
                statement();
                if (next == LWHILE)
                {
                    scaner();
                    tree[condref] = curTree;
                    exprinbrkts();
                    sopnd--;
                }
                else
                    error(wait_while_in_do_stmt);
                inloop = 0;
            }
                break;
            case LFOR:
            {
                int fromref,condref, incrref, stmtref;
                mustbe(LEFTBR, no_leftbr_in_for);
                inloop = 1;
                totree(TFor);
                fromref = curTree++;
                condref = curTree++;
                incrref = curTree++;
                stmtref = curTree++;
                if (scaner() == SEMICOLON)             // init
                    tree[fromref] = 0;
                else
                {
                    tree[fromref] = curTree;
                    expr();
                    exprassnvoid();
                    mustbe(SEMICOLON, no_semicolon_in_for);
                }
                if (scaner() == SEMICOLON)             // cond
                    tree[condref] = 0;
                else
                {
                    tree[condref] = curTree;
                    expr();
                    sopnd --;
                    mustbe(SEMICOLON, no_semicolon_in_for);
                    sopnd--;
                }
                if (scaner() == RIGHTBR)              // incr
                    tree[incrref] = 0;
                else
                {
                    tree[incrref] = curTree;
                    expr();
                    exprassnvoid();
                    mustbe(RIGHTBR, no_rightbr_in_for);
                }
                flagsemicol = 0;
                tree[stmtref] = curTree;
                statement();
                inloop = 0;
            }
                break;
            case LGOTO:
            {
                int i, flag = 1;
                mustbe(IDENT, no_ident_after_goto);
                totree(TGoto);
                for (i=0; flag && i < pgotost-1; i+=2)
                    flag = identab[ gotost[i]+1] != repr;
                if (flag)
                {                                 // первый раз встретился переход на метку, которой не было, в этом случае
                    totree(-toidentab(1));        // ссылка на identtab, стоящая после TGoto, будет отрицательной
                    gotost[pgotost++] = lastid;
                }
                else
                {
                    int id = gotost[i-2];
                    if (gotost[id+1] < 0)          // метка уже была
                    {
                        totree(id);
                        break;
                    }
                    totree(gotost[pgotost++] = id);
                }
                gotost[pgotost++] = line;
            }
                break;
            case LIF:
            {
                int thenref, elseref;
                totree(TIf);
                thenref = curTree++;
                elseref = curTree++;
                flagsemicol = 0;
                exprinbrkts();
                tree[thenref] = curTree;
                sopnd--;
                statement();
                if (next == LELSE)
                {
                    scaner();
                    tree[elseref] = curTree;
                    statement();
                }
                else
                    tree[elseref] = 0;
            }
                break;
            case LRETURN:
            {
                int ftype = modetab[functype];
                wasret = 1;
                if (next == SEMICOLON)
                {
                    if (ftype != FUNCVOID)
                        error(no_ret_in_func);
                    totree(TReturn);
                }
                else
                {
                    scaner();
                    if (ftype == FUNCVOID)
                        error(notvoidret_in_void_func);
                    totree(TReturnval);
                    expr();
                    sopnd--;
                    if (ftype == FUNCINT && ansttype == LFLOAT)
                        error(bad_type_in_ret);
                }
            }
                break;
            case LSWITCH:
            {
                flagsemicol = 0;
                totree(TSwitch);
                exprinbrkts();
                if (ansttype == LFLOAT)
                    error(float_in_switch);
                sopnd--;
                scaner();
                block(-1);
                wasdefault = 0;
                inswitch = oldinswitch;
            }
                break;
            case LWHILE:
            {
                int doref;
                inloop = 1;
                totree(TWhile);
                doref = curTree++;
                flagsemicol = 0;
                exprinbrkts();
                sopnd--;
                tree[doref] = curTree;
                statement();
                inloop = 0;
            }
                break;
            default:
                expr();
                exprassnvoid();
        }
    }
    if (flagsemicol && scaner() != SEMICOLON)
        error(no_semicolon_after_stmt);
    wasdefault = oldwasdefault;
    inswitch = oldinswitch;
}

void idorpnt(int e)
{
    point = 0;
    if (next == LMULT)
    {
        scaner();
        point = -20;
    }
    mustbe(IDENT, e);
}

void block(int b)
// если b=1, то это просто блок, b=-1 - блок в switch, иначе (b=0) - это блок функции

{
    int oldinswitch = inswitch;
    int notended = 1, i, olddispl, oldlg = lg, firstdecl;
    inswitch = b < 0;
    totree(TBegin);
    if (b)
    {
        olddispl = displ;
        curid = id;
    }
    blockflag = 0;
    while ((firstdecl = next) == LINT || firstdecl == LCHAR || firstdecl == LFLOAT)
    {
        scaner();
        int repeat = 1;
        idorpnt(after_type_must_be_ident);
        do
        {
            type = firstdecl + point;
            
            decl_id();
            
            if (next == COMMA)
            {
                scaner();
                idorpnt(wait_ident_after_comma_in_decl);
            }
            else if (next == SEMICOLON)
                 {
                     scaner();
                     repeat = 0;
                 }
                 else
                     error(def_must_end_with_semicomma);
        }
        while (repeat);
    }
    
    // кончились описания, пошли операторы до }

    do
    {
        statement();
        if (next == END)
        {
            scaner();
            notended = 0;
        }
    }
    while (notended);
    if (b)
    {
        for (i=id-4; i>=curid; i-=4)
            reprtab[ identab[i+1]+1] = identab[i];
            displ = olddispl;
    }
    inswitch = oldinswitch;
    lg = oldlg;
    totree(TEnd);
}

int modeeq(int prot, int fun)
{
    int n = modetab[prot+1], i, flag = 1;
//    printf("modeeq prot=%i fun=%i n=%i\n", prot, fun, n);
    if (modetab[prot] != modetab[fun] || n != modetab[fun+1])
        return 0;
    else
    {
        for (i=0; i<n && flag ; i++)
            flag = modetab[prot+i+2] == modetab[fun+i+2];
        return flag;
    }
}
void function_definition()
{
    int fn = identab[lastid+3], i, pred, oldrepr = repr, ftype, n, fid = lastid;
    pgotost = 0;
    functype = identab[lastid+2];
    ftype = modetab[functype];
    n = modetab[functype+1];
    wasret = 0;
    displ = 2;
    maxdispl =3;
    lg = 1;
    if ((pred = identab[lastid]) > 1)            // был прототип
        if (functype != identab[pred+2])
            error(decl_and_def_have_diff_type);
    curid = id;
    functions[fn]= curTree;
    totree(TFuncdef);
    totree(fid);
    pred = curTree++;
    repr = oldrepr;
    for (i=0; i < n; i++)
    {
        type = modetab[functype+i+2];
        repr = functions[fn+i+1];
        if (repr > 0)
            totree(toidentab(0));
        else
        {
            repr = -repr;
            toidentab(-1);
        }
    }
    block(0);
    if (ftype == FUNCVOID && tree[curTree - 1] != TReturn)
    {
        curTree--;
        totree(TReturn);
        totree(TEnd);
    }
    if ((ftype == FUNCINT || ftype == FUNCCHAR || ftype == FUNCFLOAT) && !wasret)
        error(no_ret_in_func);
    for (i=id-4; i>=curid; i-=4)
        reprtab[ identab[i+1]+1] = identab[i];
    
    for (i=0; i < pgotost-1; i+=2)
    {
        repr = identab[gotost[i]+1];
        hash = gotost[i+1];
        if (hash <0 )
            hash = - hash;
        if ( !identab[gotost[i]+2])
            error(label_not_declared);
    }
    curid = 2;                                 // все функции описываются на одном уровне
    tree[pred] = maxdispl + 1;
    lg = -1;
}

int func_declarator(int level, int func_d, int firstdecl)
{
    // на 1 уровне это может быть определением функции или предописанием, на остальных уровнях - только декларатором (без идентов)
    
    int loc_modetab[100], locmd = 0, numpar = 0, ident, maybe_fun, repeat = 1, i, wastype = 0, old;
    
    loc_modetab[0] = firstdecl;
    loc_modetab[1] = 0;
    locmd = 2;
    
    while (repeat)
    {
        if ((type = cur) == LINT || cur == LCHAR || cur == LFLOAT)
        {
            maybe_fun = 0;    // м.б. параметр-ф-ция? 0 - ничего не было, 1 - была *, 2 - была [
            ident = 0;        // = 0 - не было идента, 1 - был статический идент, 2 - был идент-параметр-функция
            wastype = 1;
            point = 0;
            if (next == LMULT)
            {
                scaner();
                type -= 20;
                maybe_fun = 1;
            }
            if (level)
            {
                if (next == IDENT)
                {
                    scaner();
                    ident = 1;
                    functions[funcnum++] = repr;
                }
            }
            else if (next == IDENT)
                error(ident_in_declarator);
            if (next == LEFTSQBR)
            {
                scaner();
                maybe_fun = 2;
                if (type < -20)
                    error(aster_with_row);
                mustbe(RIGHTSQBR, wait_right_sq_br);
                if (next == LEFTSQBR)
                {
                    scaner();
                    mustbe(RIGHTSQBR, wait_right_sq_br);
                    type -= 14;
                }
                else
                    type -= 4;
            }
        }
        else if ((type = cur) == LVOID)
        {
            wastype = 1;
            if (next != LEFTBR)
                error(par_type_void_with_nofun);
        }
        if (wastype)
        {
            numpar++;
            loc_modetab[locmd++] = type;
        
            if (next == LEFTBR)
            {
                scaner();
                mustbe(LMULT, wrong_fun_as_param);
                if (next == IDENT)
                {
                    if (level)
                    {
                        scaner();
                        if (ident == 0)
                            ident = 2;
                        else
                            error(two_idents_for_1_declarer);
                        functions[funcnum++] = -repr;
                    }
                    else
                        error(ident_in_declarator);
                }
                mustbe(RIGHTBR, no_right_br_in_paramfun);
                mustbe(LEFTBR, wrong_fun_as_param);
                scaner();
                if (maybe_fun == 1)
                    error(aster_before_func);
                else if (maybe_fun == 2)
                    error(array_before_func);

                old = func_def;
                loc_modetab[locmd-1] = func_declarator(0, 2, type-8);
                func_def = old;
            }
//            printf("level=%i ident1=%i func_d=%i loc_m=%i\n", level, ident, func_d, loc_modetab[locmd-2]);

            if (func_d == 3)
                func_d = ident > 0 ? 1 : 2;
            else if (func_d == 2 && ident > 0)
                error(wait_declarator);
            else if (func_d == 1 && ident == 0)
                error(wait_definition);
            
            if (scaner() == COMMA)
            {
                scaner();
            }
            else
                if (cur == RIGHTBR)
                    repeat = 0;
        }
        else if (cur == RIGHTBR)
        {
            repeat = 0;
            func_d = 0;
        }
             else
                 error(wrong_param_list);
    }
    func_def = func_d;
    locmd = md;
    loc_modetab[1] = numpar;
    for (i=0; i < numpar+2; i++)
        modetab[md++] = loc_modetab[i];
    old = modetab[startmode];
    while (old)
        if (modeeq(startmode+1, old+1))
            break;
        else
            old = modetab[old];
    if (old)
    {
        md = startmode + 1;
        return old+1;
    }
    modetab[md] = startmode;
    startmode = md++;
    return modetab[startmode]+1;
}

void ext_decl()
{
    do            // top level описания переменных и функций до конца файла
    {
        int repeat = 1, funrepr, first = 1;
        func_def = 3;   // func_def = 0 - (), 1 - определение функции, 2 - это предописание, 3 - не знаем или вообще не функция

        if ((type = scaner()) == LINT || type == LCHAR || type == LFLOAT || type == LVOID)
            idorpnt(after_type_must_be_ident);
        else
        {
            type = LINT;
            point = 0;
            if (cur == LMULT)
            {
                scaner();
                type -= 20;
            }
        }
        firstdecl = type;
        do                       // описываемые объекты через ',' определение функции может быть только одно, никаких ','
        {
            if (cur != IDENT)
                error(decl_must_start_from_ident_or_decl);
            
            if (firstdecl  < -20 && next == LEFTBR)
                error(aster_before_func);
            
            if (next == LEFTBR)                // определение или предописание функции
            {
                int oldfuncnum = funcnum++;
                funrepr = repr;
                scaner();
                scaner();
                type = func_declarator(first, 3, firstdecl-8);  // выкушает все параметры до ) включительно
                if (next == BEGIN)
                {
                    if (func_def == 0)
                        func_def = 1;
                }
                else if (func_def == 0)
                    func_def = 2;
                                          // теперь я точно знаю, это определение ф-ции или предописание (func_def=1 или 2)
                repr =funrepr;
                toidentab(oldfuncnum);
                
                if (next == BEGIN)
                {
                    scaner();
                    if (func_def == 2)
                        error(func_decl_req_params);
                    
                    function_definition();
                    goto ex;
                }
                else
                {
                    if (func_def == 1)
                    error(function_has_no_body);
                }
            }
            else
                if (firstdecl == LVOID)
                    error(only_functions_may_have_type_VOID);
            
// описания идентов-не-функций

            if (func_def == 3)
                decl_id();
            if (next == COMMA)
            {
                scaner();
                first = 0;
                idorpnt(wait_ident_after_comma_in_decl);
                type = firstdecl + point;
            }
            else if (next == SEMICOLON)
                 {
                     scaner();
                     repeat = 0;
                 }
                 else
                    error(def_must_end_with_semicomma);
        }
        while (repeat);
    ex: ;
    }
    while (next != LEOF);
    
    if (wasmain == 0)
        error(no_main_in_program);
}
