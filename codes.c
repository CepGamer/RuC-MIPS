//
//  codes.c
//  RuC
//
//  Created by Andrey Terekhov on 03/06/14.
//  Copyright (c) 2014 Andrey Terekhov. All rights reserved.
//

#include <stdio.h>
#include <string.h>

#include "global_vars.h"

extern void fprintf_char(FILE *f, int wchar);

void tablesandtree()
{
    int i=0, j;
    
    fprintf(output, "\n%s\n", "source");
    for (i=1; i<line; i++)
    {
        fprintf(output, "line %i) ", i);
        for (j=lines[i]; j<lines[i+1]; j++)
        {
            fprintf_char(output, source[j]);
        }
    }
    fprintf(output, "\n");
    
    fprintf(output, "\n%s\n", "identab");
    i = 2;
    while (i < id)
    {
        for (j=0; j<4; j++)
            fprintf(output, "id %i) %i\n", i +j, identab[i+j]);
        fprintf(output, "\n");
        i +=4;
    }
    
    fprintf(output, "\n%s\n", "functions");
    for (i=1; i<=funcnum; i++)
        fprintf(output, "fn %i) %i\n", i, functions[i]);
    
    fprintf(output, "\n%s\n", "modetab");
    for (i=0; i<md; i++)
        fprintf(output, "md %i) %i\n", i, modetab[i]);
    
    fprintf(output, "\n%s\n", "tree");
    for (i=0; i <= curTree; i++)
        fprintf(output, "tc %i) %i\n", i, tree[i]);
    i = 0;
    while (i < curTree)
    {
        fprintf(output, "tc %i) ", i);
        switch (tree[i++])
        {
            case TFuncdef:
                fprintf(output, "TFuncdef %i %i\n", tree[i++], tree[i++]);
                break;
            case TDeclid:
                fprintf(output, "TDeclid %i %i %i\n", tree[i++], tree[i++], tree[i++]);
                break;
            case TInit:
                fprintf(output, "TInit %i\n", tree[i++]);
                break;
            case TCondexpr:
                fprintf(output, "TCondexpr %i %i\n", tree[i++], tree[i++]);
                break;
            case TBegin:
                fprintf(output, "TBegin\n");
                break;
            case TEnd:
                fprintf(output, "TEnd\n");
                break;
            case TIf:
                fprintf(output, "TIf %i %i\n", tree[i++], tree[i++]);
                break;
            case TWhile:
                fprintf(output, "TWhile %i\n", tree[i++]);
                break;
            case TDo:
                fprintf(output, "TDo %i\n", tree[i++]);
                break;
            case TFor:
                fprintf(output, "TFor %i %i %i %i\n", tree[i++], tree[i++], tree[i++], tree[i++]);
                break;
            case TSwitch:
                fprintf(output, "TSwitch\n");
                break;
            case TCase:
                fprintf(output, "TCase\n");
                break;
            case TDefault:
                fprintf(output, "TDefault\n");
                break;
            case TBreak:
                fprintf(output, "TBreak\n");
                break;
            case TContinue:
                fprintf(output, "TContinue\n");
                break;
            case TReturn:
                fprintf(output, "TReturn\n");
                break;
            case TReturnval:
                fprintf(output, "TReturnval \n");
                break;
            case TGoto:
                fprintf(output, "TGoto %i\n", tree[i++]);
                break;
            case TIdent:
                fprintf(output, "TIdent %i\n", tree[i++]);
                break;
            case TIdenttoval:
                fprintf(output, "TIdenttoval %i\n", tree[i++]);
                break;
            case TAddrtoval:
                fprintf(output, "TAddrtoval\n");
                break;
            case TExprend:
                fprintf(output, "TExprend\n");
                break;
            case TConst:
            {
                int t = tree[i++];
                memcpy(&numfloat, &t, 4);
                fprintf(output, "TConst %i %f\n", t, numfloat);
            }
                break;
            case TSliceident:
                fprintf(output, "TSliceident %i\n", tree[i++]);
                break;
            case TSlice:
                fprintf(output, "TSlice\n");
                break;
            case TCall1:
                fprintf(output, "TCall1 %i\n", tree[i++]);
                break;
            case TCall2:
                fprintf(output, "TCall2 %i\n", tree[i++]);
                break;
            case TLabel:
                fprintf(output, "TLabel %i\n", tree[i++]);
                break;
            case TPrint:
                fprintf(output, "TPrint %i\n", tree[i++]);
                break;
            case TPrintid:
                fprintf(output, "TPrintid %i\n", tree[i++]);
                break;
            case TGetid:
                fprintf(output, "TGetid %i\n", tree[i++]);
                break;
            case SETMOTOR:
                fprintf(output, "Setmotor\n");
                break;
            case SLEEP:
                fprintf(output, "Sleep\n");

            default:
                fprintf(output, "TOper %i\n", tree[i-1]);
        }
    }
}

void tablesandcode()
{
    int i=0, j;
    
    fprintf(output, "\n%s\n", "source");
    for (i=1; i<line; i++)
    {
        fprintf(output, "line %i) ", i);
        for (j=lines[i]; j<lines[i+1]; j++)
        {
            fprintf_char(output, source[j]);
        }
    }
    fprintf(output, "\n");
/*
    fprintf(output, "\n%s\n", "identab");
    i = 2;
    while (i < id)
    {
        for (j=0; j<4; j++)
            fprintf(output, "id %i) %i\n", i +j, identab[i+j]);
        fprintf(output, "\n");
        i +=4;
    }
    
    fprintf(output, "\n%s\n", "functions");
    for (i=1; i<=funcnum; i++)
        fprintf(output, "fn %i) %i\n", i, functions[i]);
    
    fprintf(output, "\n%s\n", "modetab");
    for (i=0; i<md; i++)
        fprintf(output, "md %i) %i\n", i, modetab[i]);
*/    
    fprintf(output, "%s\n", "mem");
    i = 0;
    while (i < pc)
    {
        fprintf(output, "pc %i) ", i);
        switch (mem[i++])
        {
            case PRINT:
                fprintf(output, "PRINT ");
                fprintf(output, "%i\n", mem[i++]);
                break;
            case PRINTID:
                fprintf(output, "PRINTID ");
                fprintf(output, "%i\n", mem[i++]);
                break;
            case GETID:
                fprintf(output, "GETID ");
                fprintf(output, "%i\n", mem[i++]);
                break;
            case SETMOTOR:
                fprintf(output, "SETMOTOR\n");
                break;
            case SLEEP:
                fprintf(output, "SLEEP\n");
                break;
            case GETDIGSENSORC:
                fprintf(output, "GETDIGSENSOR\n");
                break;
            case GETANSENSORC:
                fprintf(output, "GETANSENSOR\n");
                break;
            case ABSC:
                fprintf(output, "ABS\n");
                break;
            case ABSIC:
                fprintf(output, "ABSI\n");
                break;
            case SQRTC:
                fprintf(output, "SQRT\n");
                break;
            case EXPC:
                fprintf(output, "EXP\n");
                break;
            case SINC:
                fprintf(output, "SIN\n");
                break;
            case COSC:
                fprintf(output, "COS\n");
                break;
            case LOGC:
                fprintf(output, "LOG\n");
                break;
            case LOG10C:
                fprintf(output, "LOG10\n");
                break;
            case ASINC:
                fprintf(output, "ASIN\n");
                break;
                
            case DECX:
                fprintf(output, "DECX\n");
                break;
            case DEFARR:
                fprintf(output, "DEFARR ");
                fprintf(output, "%i\n", mem[i++]);
                break;
            case DEFARR2:
                fprintf(output, "DEFARR2 ");
                fprintf(output, "%i\n", mem[i++]);
                break;
            case NOP:
                fprintf(output, "NOP\n");
                break;
            case LI:
                fprintf(output, "LI ");
                memcpy(&numfloat, &mem[i], sizeof(int));
                fprintf(output, "%i %f\n", mem[i++], numfloat);
                break;
            case LOAD:
                fprintf(output, "LOAD ");
                fprintf(output, "%i\n", mem[i++]);
                break;
            case LAT:
                fprintf(output,"L@\n");
                break;
            case LA:
                fprintf(output, "LA ");
                fprintf(output, "%i\n", mem[i++]);
                break;
                
            case LOGOR:
                fprintf(output, "||\n");
                break;
            case LOGAND:
                fprintf(output, "&&\n");
                break;
            case ORASS:
                fprintf(output, "|= ");
                fprintf(output, "%i\n", mem[i++]);
                break;
            case ORASSAT:
                fprintf(output, "|=@\n");
                break;
            case ORASSV:
                fprintf(output, "|=V ");
                fprintf(output, "%i\n", mem[i++]);
                break;
            case ORASSATV:
                fprintf(output, "|=@V\n");
                break;
            case LOR:
                fprintf(output, "|\n");
                break;
            case EXORASS:
                fprintf(output, "^= ");
                fprintf(output, "%i\n", mem[i++]);
                break;
            case EXORASSAT:
                fprintf(output, "^=@\n");
                break;
            case EXORASSV:
                fprintf(output, "^=V ");
                fprintf(output, "%i\n", mem[i++]);
                break;
            case EXORASSATV:
                fprintf(output, "^=@V\n");
                break;
            case LEXOR:
                fprintf(output, "^\n");
                break;
            case ANDASS:
                fprintf(output, "&= ");
                fprintf(output, "%i\n", mem[i++]);
                break;
            case ANDASSAT:
                fprintf(output, "&=@\n");
                break;
            case ANDASSV:
                fprintf(output, "&=V ");
                fprintf(output, "%i\n", mem[i++]);
                break;
            case ANDASSATV:
                fprintf(output, "&=@V\n");
                break;
            case LAND:
                fprintf(output, "&\n");
                break;
                
            case EQEQ:
                fprintf(output, "==\n");
                break;
            case NOTEQ:
                fprintf(output, "!=\n");
                break;
            case LLT:
                fprintf(output, "<\n");
                break;
            case LGT:
                fprintf(output, ">\n");
                break;
            case LLE:
                fprintf(output, "<=\n");
                break;
            case LGE:
                fprintf(output, ">=\n");
                break;
            case EQEQR:
                fprintf(output, "==f\n");
                break;
            case NOTEQR:
                fprintf(output, "!=f\n");
                break;
            case LLTR:
                fprintf(output, "<f\n");
                break;
            case LGTR:
                fprintf(output, ">f\n");
                break;
            case LLER:
                fprintf(output, "<=f\n");
                break;
            case LGER:
                fprintf(output, ">=f\n");
                break;
                
            case SHRASS:
                fprintf(output, ">>= ");
                fprintf(output, "%i\n", mem[i++]);
                break;
            case SHRASSAT:
                fprintf(output, ">>=@\n");
                break;
            case SHRASSV:
                fprintf(output, ">>=V ");
                fprintf(output, "%i\n", mem[i++]);
                break;
            case SHRASSATV:
                fprintf(output, ">>=@V\n");
                break;
            case LSHR:
                fprintf(output, ">>\n");
                break;
            case SHLASS:
                fprintf(output, "<<= ");
                fprintf(output, "%i\n", mem[i++]);
                break;
            case SHLASSAT:
                fprintf(output, "<<=@\n");
                break;
            case SHLASSV:
                fprintf(output, "<<=V ");
                fprintf(output, "%i\n", mem[i++]);
                break;
            case SHLASSATV:
                fprintf(output, "<<=@V\n");
                break;
            case LSHL:
                fprintf(output, "<<\n");
                break;
            case ASS:
                fprintf(output, "= ");
                fprintf(output, "%i\n", mem[i++]);
                break;
            case ASSAT:
                fprintf(output, "=@\n");
                break;
            case ASSV:
                fprintf(output, "=V ");
                fprintf(output, "%i\n", mem[i++]);
                break;
            case ASSATV:
                fprintf(output, "=@V\n");
                break;

                
            case PLUSASS:
                fprintf(output, "+= ");
                fprintf(output, "%i\n", mem[i++]);
                break;
            case PLUSASSAT:
                fprintf(output, "+=@\n");
                break;
            case PLUSASSV:
                fprintf(output, "+=V ");
                fprintf(output, "%i\n", mem[i++]);
                break;
            case PLUSASSATV:
                fprintf(output, "+=@V\n");
                break;

            case LPLUS:
                fprintf(output, "+\n");
                break;
            case MINUSASS:
                fprintf(output, "-= ");
                fprintf(output, "%i\n", mem[i++]);
                break;
            case MINUSASSAT:
                fprintf(output, "-=@\n");
                break;
            case MINUSASSV:
                fprintf(output, "-=V ");
                fprintf(output, "%i\n", mem[i++]);
                break;
            case MINUSASSATV:
                fprintf(output, "-=@V\n");
                break;

            case LMINUS:
                fprintf(output, "-\n");
                break;
            case MULTASS:
                fprintf(output, "*= ");
                fprintf(output, "%i\n", mem[i++]);
                break;
            case MULTASSAT:
                fprintf(output, "*=@\n");
                break;
            case MULTASSV:
                fprintf(output, "*=V ");
                fprintf(output, "%i\n", mem[i++]);
                break;
            case MULTASSATV:
                fprintf(output, "*=@V\n");
                break;

            case LMULT:
                fprintf(output, "*\n");
                break;
            case DIVASS:
                fprintf(output, "/= ");
                fprintf(output, "%i\n", mem[i++]);
                break;
            case DIVASSAT:
                fprintf(output, "/=@\n");
                break;
            case DIVASSV:
                fprintf(output, "/=V ");
                fprintf(output, "%i\n", mem[i++]);
                break;
            case DIVASSATV:
                fprintf(output, "/=@V\n");
                break;
            case LDIV:
                fprintf(output, "/\n");
                break;
                
            case PLUSASSR:
                fprintf(output, "+=f ");
                fprintf(output, "%i\n", mem[i++]);
                break;
            case PLUSASSATR:
                fprintf(output, "+=@f\n");
                break;
            case PLUSASSRV:
                fprintf(output, "+=fV ");
                fprintf(output, "%i\n", mem[i++]);
                break;
            case PLUSASSATRV:
                fprintf(output, "+=@fV\n");
                break;
            case LPLUSR:
                fprintf(output, "+f\n");
                break;
            case MINUSASSR:
                fprintf(output, "-=f ");
                fprintf(output, "%i\n", mem[i++]);
                break;
            case MINUSASSATR:
                fprintf(output, "-=@f\n");
                break;
            case MINUSASSRV:
                fprintf(output, "-=fV ");
                fprintf(output, "%i\n", mem[i++]);
                break;
            case MINUSASSATRV:
                fprintf(output, "-=@fV\n");
                break;
            case LMINUSR:
                fprintf(output, "-f\n");
                break;
            case MULTASSR:
                fprintf(output, "*=f ");
                fprintf(output, "%i\n", mem[i++]);
                break;
            case MULTASSATR:
                fprintf(output, "*=@f\n");
                break;
            case MULTASSRV:
                fprintf(output, "*=fV ");
                fprintf(output, "%i\n", mem[i++]);
                break;
            case MULTASSATRV:
                fprintf(output, "*=@fV\n");
                break;
            case LMULTR:
                fprintf(output, "*f\n");
                break;
            case DIVASSR:
                fprintf(output, "/=f ");
                fprintf(output, "%i\n", mem[i++]);
                break;
            case DIVASSATR:
                fprintf(output, "/=@f\n");
                break;
            case DIVASSRV:
                fprintf(output, "/=fV ");
                fprintf(output, "%i\n", mem[i++]);
                break;
            case DIVASSATRV:
                fprintf(output, "/=@fV\n");
                break;
            case LDIVR:
                fprintf(output, "/f\n");
                break;
                
            case REMASS:
                fprintf(output, "%%= ");
                fprintf(output, "%i\n", mem[i++]);
                break;
            case REMASSAT:
                fprintf(output, "%%=@\n");
                break;
            case REMASSV:
                fprintf(output, "%%=V ");
                fprintf(output, "%i\n", mem[i++]);
                break;
            case REMASSATV:
                fprintf(output, "%%=@V\n");
                break;
            case LREM:
                fprintf(output, "%%\n");
                break;
            case CALL1:
                fprintf(output, "CALL1\n");
                break;
            case CALL2:
                fprintf(output, "CALL2 ");
                fprintf(output, "%i\n", mem[i++]);
                break;
            case STOP:
                fprintf(output, "STOP\n");
                break;
            case _RETURN:
                fprintf(output, "RETURN\n");
                break;
            case RETURNV:
                fprintf(output, "RETURNV\n");
                break;
            case B:
                fprintf(output, "B ");
                fprintf(output, "%i\n", mem[i++]);
                break;
            case STRING:
            {
                int j, n;
                fprintf(output,"STRING ");
                fprintf(output, "%i\n", mem[i++]);
                fprintf(output, "n=%i\n", n = mem[i++]);
                for (j=0; j<n; j++)
                    fprintf(output, "%c\n", mem[i++]);
                fprintf(output,"%i\n", mem[i++]);
                break;
            }
                break;
            case BE0:
                fprintf(output, "BE0 ");
                fprintf(output, "%i\n", mem[i++]);
                break;
            case BNE0:
                fprintf(output, "BNE0 ");
                fprintf(output, "%i\n", mem[i++]);
                break;
            case SLICE:
                fprintf(output, "SLICE\n");
                break;
            case ASSARR:
                fprintf(output, "ASSARR ");
                fprintf(output, "%i ", mem[i++]);
                fprintf(output, "%i\n", mem[i++]);
                break;
            case ASSARR2:
                fprintf(output, "ASSARR2 ");
                fprintf(output, "%i ", mem[i++]);
                fprintf(output, "%i\n", mem[i++]);
                break;
            case WIDEN:
                fprintf(output, "WIDEN\n");
                break;
            case WIDEN1:
                fprintf(output, "WIDEN1\n");
                break;
            case DOUBLE:
                fprintf(output, "DOUBLE\n");
                break;
            case LNOT:
                fprintf(output, "BITNOT\n");
                break;
            case LOGNOT:
                fprintf(output, "NOT\n");
                break;
            case UNMINUS:
                fprintf(output, "UNMINUS\n");
                break;
            case UNMINUSR:
                fprintf(output, "UNMINUSf\n");
                break;
                
            case FUNCBEG:
                fprintf(output, "FUNCBEG ");
                fprintf(output, "%i ", mem[i++]);
                fprintf(output, "%i\n", mem[i++]);
                break;
                
                
            default:
                fprintf(output, "%i\n", mem[i-1]);
                break;
        }
    }
    
}

void cd(int pcl)
{
    int i = pcl;
    while (i < pc)
    {
        printf("pc %i) ", i);
        switch (mem[i++])
        {
            case PRINT:
                printf("PRINT ");
                printf("%i\n", mem[i++]);
                break;
            case PRINTID:
                printf("PRINTID ");
                printf("%i\n", mem[i++]);
                break;
            case GETID:
                printf("GETID ");
                printf("%i\n", mem[i++]);
                break;
            case SETMOTOR:
                printf("SETMOTOR\n");
                break;
            case SLEEP:
                printf("SLEEP\n");
                break;
            case GETDIGSENSORC:
                printf("GETDIGSENSOR\n");
                break;
            case GETANSENSORC:
                printf("GETANSENSOR\n");
                break;
            case ABSC:
                printf("ABS\n");
                break;
            case ABSIC:
                printf("ABSI\n");
                break;
            case SQRTC:
                printf("SQRT\n");
                break;
            case EXPC:
                printf("EXP\n");
                break;
            case SINC:
                printf("SIN\n");
                break;
            case COSC:
                printf("COS\n");
                break;
            case LOGC:
                printf("LOG\n");
                break;
            case LOG10C:
                printf("LOG10\n");
                break;
            case ASINC:
                printf("ASIN\n");
                break;
                
            case DECX:
                printf("DECX\n");
                break;
            case NOP:
                printf("NOP\n");
                break;
            case DEFARR:
                printf("DEFARR ");
                printf("%i\n", mem[i++]);
                break;
            case DEFARR2:
                printf("DEFARR2 ");
                printf("%i\n", mem[i++]);
                break;
            case LI:
                printf("LI ");
                memcpy(&numfloat, &mem[i], sizeof(num));
                printf("%i %f\n", mem[i++], numfloat);
                break;
            case LOAD:
                printf("LOAD ");
                printf("%i\n", mem[i++]);
                break;
            case LAT:
                printf("L@\n");
                break;
            case LA:
                printf("LA ");
                printf("%i\n", mem[i++]);
                break;
            case LOGOR:
                printf("||\n");
                break;
            case LOGAND:
                printf("&&\n");
                break;
            case ORASS:
                printf("|= ");
                printf("%i\n", mem[i++]);
                break;
            case ORASSAT:
                printf("|=@\n");
                break;
            case LOR:
                printf("|\n");
                break;
            case EXORASS:
                printf("^= ");
                printf("%i\n", mem[i++]);
                break;
            case EXORASSAT:
                printf("^=@\n");
                break;
            case LEXOR:
                printf("^\n");
                break;
            case ANDASS:
                printf("&= ");
                printf("%i\n", mem[i++]);
                break;
            case ANDASSAT:
                printf("&=@\n");
                break;
            case LAND:
                printf("&\n");
                break;
            case EQEQ:
                printf("==\n");
                break;
            case NOTEQ:
                printf("!=\n");
                break;
            case LLT:
                printf("<\n");
                break;
            case LGT:
                printf(">\n");
                break;
            case LLE:
                printf("<=\n");
                break;
            case LGE:
                printf(">=\n");
                break;
            case EQEQR:
                printf("==f\n");
                break;
            case NOTEQR:
                printf("!=f\n");
                break;
            case LLTR:
                printf("<f\n");
                break;
            case LGTR:
                printf(">f\n");
                break;
            case LLER:
                printf("<=f\n");
                break;
            case LGER:
                printf(">=f\n");
                break;
                
            case SHRASS:
                printf(">>= ");
                printf("%i\n", mem[i++]);
                break;
            case SHRASSAT:
                printf(">>=@\n");
                break;
            case LSHR:
                printf(">>\n");
                break;
            case SHLASS:
                printf("<<= ");
                printf("%i\n", mem[i++]);
                break;
            case SHLASSAT:
                printf("<<=@\n");
                break;
            case LSHL:
                printf("<<\n");
                break;
            case ASS:
                printf("= ");
                printf("%i\n", mem[i++]);
                break;
            case ASSAT:
                printf("=@\n");
                break;
            case PLUSASS:
                printf("+= ");
                printf("%i\n", mem[i++]);
                break;
            case PLUSASSAT:
                printf("+=@\n");
                break;
            case LPLUS:
                printf("+\n");
                break;
            case MINUSASS:
                printf("-= ");
                printf("%i\n", mem[i++]);
                break;
            case MINUSASSAT:
                printf("-=@\n");
                break;
            case LMINUS:
                printf("-\n");
                break;
            case MULTASS:
                printf("*= ");
                printf("%i\n", mem[i++]);
                break;
            case MULTASSAT:
                printf("*=@\n");
                break;
            case LMULT:
                printf("*\n");
                break;
            case DIVASS:
                printf("/= ");
                printf("%i\n", mem[i++]);
                break;
            case DIVASSAT:
                printf("/=@\n");
                break;
            case LDIV:
                printf("/\n");
                break;
            case PLUSASSR:
                printf("+=f ");
                printf("%i\n", mem[i++]);
                break;
            case PLUSASSATR:
                printf("+=@f\n");
                break;
            case LPLUSR:
                printf("+f\n");
                break;
            case MINUSASSR:
                printf("-=f ");
                printf("%i\n", mem[i++]);
                break;
            case MINUSASSATR:
                printf("-=@f\n");
                break;
            case LMINUSR:
                printf("-f\n");
                break;
            case MULTASSR:
                printf("*=f ");
                printf("%i\n", mem[i++]);
                break;
            case MULTASSATR:
                printf("*=@f\n");
                break;
            case LMULTR:
                printf("*f\n");
                break;
            case DIVASSR:
                printf("/=f ");
                printf("%i\n", mem[i++]);
                break;
            case DIVASSATR:
                printf("/=@f\n");
                break;
            case LDIVR:
                printf("/f\n");
                break;
                
            case REMASS:
                printf("%%= ");
                printf("%i\n", mem[i++]);
                break;
            case REMASSAT:
                printf("%%=@\n");
                break;
            case LREM:
                printf("%%\n");
                break;
            case CALL1:
                printf("CALL1\n");
                break;
            case CALL2:
                printf("CALL2 ");
                printf("%i\n", mem[i++]);
                break;
            case STOP:
                printf("STOP\n");
                break;
            case _RETURN:
                printf("RETURN\n");
                break;
            case RETURNV:
                printf("RETURNV\n");
                break;
            case B:
                printf("B ");
                printf("%i\n", mem[i++]);
                break;
            case STRING:
            {
                int j, n;
                printf("STRING ");
                printf("%i\n", mem[i++]);
                printf("n=%i\n", n = mem[i++]);
                for (j=0; j<n; j++)
                    printf("%c\n", mem[i++]);
                printf("%i\n", mem[i++]);
                break;
            }
            case BE0:
                printf("BE0 ");
                printf("%i\n", mem[i++]);
                break;
            case BNE0:
                printf("BNE0 ");
                printf("%i\n", mem[i++]);
                break;
            case SLICE:
                printf("SLICE\n");
                break;
            case ASSARR:
                printf("ASSARR ");
                printf("%i ", mem[i++]);
                printf("%i\n", mem[i++]);
                break;
            case ASSARR2:
                printf("ASSARR2 ");
                printf("%i ", mem[i++]);
                printf("%i\n", mem[i++]);
                break;
            case WIDEN:
                printf("WIDEN\n");
                break;
            case WIDEN1:
                printf("WIDEN1\n");
                break;
            case DOUBLE:
                printf("DOUBLE\n");
                break;
            case INC:
                printf("INC ");
                printf("%i\n", mem[i++]);
                break;
            case DEC:
                printf("DEC ");
                printf("%i\n", mem[i++]);
                break;
            case POSTINC:
                printf("POSTINC ");
                printf("%i\n", mem[i++]);
                break;
            case POSTDEC:
                printf("POSTDEC ");
                printf("%i\n", mem[i++]);
                break;
            case INCAT:
                printf("INC@\n");
                break;
            case DECAT:
                printf("DEC@\n");
                break;
            case POSTINCAT:
                printf("POSTINC@\n");
                break;
            case POSTDECAT:
                printf("POSTDEC@\n");
                break;
            case INCR:
                printf("INCf ");
                printf("%i\n", mem[i++]);
                break;
            case DECR:
                printf("DECf ");
                printf("%i\n", mem[i++]);
                break;
            case POSTINCR:
                printf("POSTINCf ");
                printf("%i\n", mem[i++]);
                break;
            case POSTDECR:
                printf("POSTDECf ");
                printf("%i\n", mem[i++]);
                break;
            case INCATR:
                printf("INC@f\n");
                break;
            case DECATR:
                printf("DEC@f\n");
                break;
            case POSTINCATR:
                printf("POSTINC@f\n");
                break;
            case POSTDECATR:
                printf("POSTDEC@f\n");
                break;
            case INCV:
                printf("INCV ");
                printf("%i\n", mem[i++]);
                break;
            case DECV:
                printf("DECV ");
                printf("%i\n", mem[i++]);
                break;
            case POSTINCV:
                printf("POSTINCV ");
                printf("%i\n", mem[i++]);
                break;
            case POSTDECV:
                printf("POSTDECV ");
                printf("%i\n", mem[i++]);
                break;
            case INCATV:
                printf("INC@V\n");
                break;
            case DECATV:
                printf("DEC@V\n");
                break;
            case POSTINCATV:
                printf("POSTINC@V\n");
                break;
            case POSTDECATV:
                printf("POSTDEC@V\n");
                break;
            case INCRV:
                printf("INCfV ");
                printf("%i\n", mem[i++]);
                break;
            case DECRV:
                printf("DECfV");
                printf("%i\n", mem[i++]);
                break;
            case POSTINCRV:
                printf("POSTINCfV ");
                printf("%i\n", mem[i++]);
                break;
            case POSTDECRV:
                printf("POSTDECfV ");
                printf("%i\n", mem[i++]);
                break;
            case INCATRV:
                printf("INC@fV\n");
                break;
            case DECATRV:
                printf("DEC@fV\n");
                break;
            case POSTINCATRV:
                printf("POSTINC@fV\n");
                break;
            case POSTDECATRV:
                printf("POSTDEC@fV\n");
                break;

            case LNOT:
                printf("BITNOT\n");
                break;
            case LOGNOT:
                printf("NOT\n");
                break;
            case UNMINUS:
                printf("UNMINUS\n");
                break;
            case UNMINUSR:
                printf("UNMINUSf\n");
                break;
            case FUNCBEG:
                printf("FUNCBEG ");
                printf("%i ", mem[i++]);
                printf("%i\n", mem[i++]);
                break;
                
            default:
                printf("\n wrong cop %i\n", mem[i-1]);
                break;
        }
    }
}
