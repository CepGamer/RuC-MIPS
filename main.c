//  RuC2
//
//  Created by Andrey Terekhov on 11/Jan/15.
//  Copyright (c) 2015 Andrey Terekhov. All rights reserved.
//
// http://www.lysator.liu.se/c/ANSI-C-grammar-y.html

char* name = "../Regress tests/test.c";

#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <stdlib.h>

#include "Defs.h"

// Определение глобальных переменных

FILE *input, *output;
float numfloat;
int line=0, charnum=1, cur, next, next1, num, numr, hash, repr, keywordsnum,
source[SOURCESIZE], lines[LINESSIZE];
int nextchar, curchar, func_def;
int hashtab[256], reprtab[MAXREPRTAB], rp = 1, identab[MAXIDENTAB], id = 2, modetab[MAXMODSIZE], md = 2, startmode = 1;
int stack[100], stackop[100], stackoperands[100], ansttype,
sp=0, sopnd=-1, aux=0, lastid, curid = 2, lg=-1, displ=-2, maxdispl = 3, maxdisplg = 3, type, op, firstdecl, point;
int instring = 0, inswitch = 0, inloop = 0, lexstr[MAXSTRINGL+1];

int tree[MAXTREESIZE], tc=0, mem[MAXMEMSIZE], pc=0, functions[FUNCSIZE], funcnum = 2, functype, kw = 0, blockflag = 1,
entry, wasmain = 0, wasret, wasdefault, notrobot = 1;
int adcont, adbreak, adcase;
int gotost[1000], pgotost;
int anst, anstdispl, ansttype;              // anst = VAL  - значение на стеке
int g, l, x;                                // anst = ADDR - на стеке адрес значения
                                            // anst = IDENT- значение в статике, в anstdisl смещение отl или g
                                            // в ansttype всегда тип возвращаемого значения 

                                            // box - запрос на значение
                                            // F, если значение можно разместить как угодно,
                                            // VAL, если значение нужно поместить на верхушке стека
                                            // DECX, если значения не нужно
                                            // opassn+11, если нужно присвоить значение по адресу из верхушки стека,
                                            // opassn,    если нужно присвоить переменной

extern void tablesandcode();
extern void tablesandtree();
extern void import();
extern int  getnext();
extern int  nextch();
extern int  scan();
extern void error(int ernum);
extern void codegen();
extern void ext_decl();
extern void compile_mips();

int main()
{
    int i;
    
    for (i=0; i<256; i++)
        hashtab[i] = 0;
    output = fopen("../tree.txt", "wt");
    
// занесение ключевых слов в reprtab
    keywordsnum = 1;
    input =  fopen("../keywords.txt", "r");
    getnext();
    nextch();
    while (scan() != LEOF)
        ;
    fclose(input);
    
    input =  fopen(name, "r");
    if (input == NULL)
    {
        printf(" file not found %s\n", name);
        exit(1);
    }
    modetab[1] = 0;
    keywordsnum = 0;
    lines[line = 1] = 1;
    charnum = 1;
    kw = 1;
    tc = 0;
    getnext();
    nextch();
    next = scan();
    
    ext_decl();
    
    lines[line+1] = charnum;
    tablesandtree();
    fclose(output);
    
    output = fopen("../codes.txt", "wt");
    
    codegen();

    tablesandcode();

    fclose(input);
    fclose(output);
    
    output = fopen("../export.txt", "wt");
    fprintf(output, "%i %i %i %i %i\n", pc, funcnum, id, rp, maxdisplg);
    
    for (i=0; i<pc; i++)
    {
        fprintf(output, "%i ", mem[i]);
    }
    fprintf(output, "\n");

    for (i=0; i<funcnum; i++)
        fprintf(output, "%i ", functions[i]);
    fprintf(output, "\n");
    
    for (i=0; i<id; i++)
        fprintf(output, "%i ", identab[i]);
    fprintf(output, "\n");

    for (i=0; i<rp; i++)
        fprintf(output, "%i ", reprtab[i]);
    fprintf(output, "\n");
    
    fclose(output);

    if (notrobot)
        import();

    compile_mips();

    return 0;
}

