//
//  import.c
//
//  Created by Andrey Terekhov on 2/25/14.
//  Copyright (c) 2014 Andrey Terekhov. All rights reserved.
//

//#define ROBOT 1
//#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "global_vars.h"

#define I2CBUFFERSIZE 50

#define index_out_of_range  1
#define wrong_kop           2
#define wrong_arr_init      3
#define wrong_motor_num     4
#define wrong_motor_pow     5
#define wrong_digsensor_num 6
#define wrong_ansensor_num  7
#define wrong_robot_com     8
#define wrong_number_of_elems 9

#ifdef ROBOT
FILE *f1, *f2;   // файлы цифровых датчиков
const char* JD1 = "/sys/devices/platform/da850_trik/sensor_d1";
const char* JD2 = "/sys/devices/platform/da850_trik/sensor_d2";

int rungetcommand(const char *command)
{
    FILE *fp;
    int x = -1;
    char path[100] = {'\0'};
    
    /* Open the command for reading. */
    fp = popen(command, "r");
    if (fp == NULL)
        runtimeerr(wrong_robot_com, 0,0);
    
    /* Read the output a line at a time - output it. */
    while (fgets(path, sizeof(path)-1, fp) != NULL)
    {
        x = strtol(path, NULL, 16);
        printf("[%s] %d\n", path, x);
    }
    pclose(fp);
    return x;                   // ??????
}

#endif

void printf_char(int wchar)
{
    if (wchar<128)
        printf("%c", wchar);
    else
    {
        unsigned char first = (wchar >> 6) | /*0b11000000*/ 0xC0;
        unsigned char second = (wchar & /*0b111111*/ 0x3F) | /*0b10000000*/ 0x80;

        printf("%c%c", first, second);
    }
}

void fprintf_char(FILE *f, int wchar)
{    if (wchar<128)
    fprintf(f, "%c", wchar);
else
{
    unsigned char first = (wchar >> 6) | /*0b11000000*/ 0xC0;
    unsigned char second = (wchar & /*0b111111*/ 0x3F) | /*0b10000000*/ 0x80;
    
    fprintf(f, "%c%c", first, second);
}
}

void printident(int r)
{
    r += 2;
    do
        printf_char(reprtab[r++]);
    while (reprtab[r] != 0);
    printf(" ");
}

int dspl(int d)
{
    return d < 0 ? g-d : l+d;
}

int dsp()
{
    return dspl(mem[pc++]);
}

void runtimeerr(int e, int i, int r)
{
    switch (e)
    {
        case index_out_of_range:
            printf("индекс %i за пределами границ массива %i\n", i, r);
            break;
        case wrong_kop:
            printf("команду %i я пока не реализовал\n", i);
            break;
        case wrong_arr_init:
            printf("массив с %i элементами инициализируется %i значениями\n", i, r);
            break;
        case wrong_motor_num:
            printf("номер силового мотора %i, а должен быть от 1 до 4\n", i);
            break;
        case wrong_motor_pow:
            printf("задаваемая мощность мотора %i равна %i, а должна быть от -100 до 100\n", i, r);
            break;
        case wrong_digsensor_num:
            printf("номер цифрового сенсора %i, а должен быть 1 или 2\n", i);
            break;
        case wrong_ansensor_num:
            printf("номер аналогового сенсора %i, а должен быть от 1 до 6\n", i);
            break;
        case wrong_robot_com:
            printf("робот не может исполнить команду\n");
            break;
        case wrong_number_of_elems:
            printf("количиство элементов в массиве по каждому измерению должно быть положительным, а тут %i", r);
            
            
        default:
            break;
    }
    exit(3);
}

void prmem()
{
    int i;
    printf("mem=\n");
    for (i=g; i<=x; i++)
        printf("%i ) %i\n",i, mem[i]);
    printf("\n");
    
}

int auxpr(int r, int t)
{
    float rf;
    int j, flag=0;
    for (j=0; j<mem[r-1]; j++)
        if (t == ROWOFINT)
            printf(" %i", mem[r+j]);
        else if (t == ROWOFCHAR)
            printf("%c", mem[r+j]);
        else if(t == ROWOFFLOAT)
        {
            memcpy(&rf, &mem[r+j], sizeof(int));
            printf(" %f", rf);
        }
        else
            flag = 1;
    return flag;
}

void auxprint(int r, int t)
{
    float rf;
    if (t == LINT)
        printf(" %i", r);
    else if (t == LCHAR)
        printf("%c", r);
    else if (t == LFLOAT)
    {
        memcpy(&rf, &r, sizeof(int));
        printf(" %f", rf);
    }
    else
    {
        int i;
        if (auxpr(r, t))
            // двухмерные массивы
            for (i=0; i<mem[r-1]; i++)
            {
                auxpr(mem[r+i], t+10);
                printf("\n");
            }
    }
    printf("\n");
}

int auxgt(int *r, int t)
{
    float rf;
    int j, flag=0;
    for (j=0; j<mem[(*r)-1]; j++)
        if (t == ROWOFINT)
            scanf(" %i", &mem[*r+j]);
        else if (t == ROWOFCHAR)
        {
            mem[*r+j] = 0;
            scanf("%c", &mem[*r+j]);
        }
        else if(t == ROWOFFLOAT)
        {
            scanf(" %f", &rf);
            memcpy(&mem[*r+j], &rf, sizeof(num));
        }
        else
            flag = 1;
    return flag;
}

void auxget(int *r, int t)
{
    float rf;
    if (t == LINT)
        scanf(" %i", r);
    else if (t == LCHAR)
    {
        r = 0;
        scanf("%c", r);
    }
    else if (t == LFLOAT)
    {
        scanf(" %f", &rf);
        memcpy(&r, &rf, sizeof(int));
    }
    else
    {
        int i;
        if (auxgt(r, t))
            // двухмерные массивы
            for (i=0; i<mem[*r-1]; i++)
                auxgt(&mem[*r+i], t+10);
    }
}


void import()
{
    FILE *input;
    int i,r, n, flagstop = 1, entry, num;
    float lf, rf;
    char i2ccommand[I2CBUFFERSIZE];
    
#ifdef ROBOT
    f1 = fopen(JD1, "r");                       // файлы цифровых датчиков
    f2 = fopen(JD2, "r");
    printf("stage 1\n");
    system("i2cset -y 2 0x48 0x10 0x1000 w");   // инициализация силовых моторов
    system("i2cset -y 2 0x48 0x11 0x1000 w");
    system("i2cset -y 2 0x48 0x12 0x1000 w");
    system("i2cset -y 2 0x48 0x13 0x1000 w");
#endif
    
    input = fopen("../export.txt", "r");
    
    fscanf(input, "%i %i %i %i %i\n", &pc, &funcnum, &id, &rp, &maxdisplg);

    for (i=0; i<pc; i++)
    {
        fscanf(input, "%i ", &mem[i]);
    }

    for (i=0; i<funcnum; i++)
        fscanf(input, "%i ", &functions[i]);
    
    for (i=0; i<id; i++)
        fscanf(input, "%i ", &identab[i]);
    
    for (i=0; i<rp; i++)
        fscanf(input, "%i ", &reprtab[i]);
    
    fclose(input);
    
    l = g = pc;
    mem[g] = mem[g+1] = 0;
    x = g + maxdisplg;
    pc = 0;
    
    while (flagstop)
    {
        memcpy(&rf, &mem[x], sizeof(num));
//        printf("pc=%i mem[pc]=%i rf=%f\n", pc, mem[pc], rf);
        switch (mem[pc++])
        {
            case STOP:
                flagstop = 0;
                break;
#ifdef ROBOT
            case SETMOTOR:
                r = mem[x--];
                n = mem[x--];
                if (n < 1 || n > 4)
                    runtimeerr(wrong_motor_num, n, 0);
                if (r < -100 || r > 100)
                    runtimeerr(wrong_motor_pow, n, r);
                memset(i2ccommand, '\0', I2CBUFFERSIZE);
                printf("i2cset -y 2 0x48 0x%x 0x%x b\n", 0x14 + n - 1, r);
                snprintf(i2ccommand, I2CBUFFERSIZE, "i2cset -y 2 0x48 0x%x 0x%x b", 0x14 + n - 1, r);
                system(i2ccommand);
                break;
            case CGETDIGSENSOR:
                n = mem[x];
                if (n < 1 || n > 2)
                    runtimeerr(wrong_digsensor_num, n, 0);
                if (n == 1)
                    fscanf(f1, "%i", &i);
                else
                    fscanf(f2, "%i", &i);
                mem[x] = i;
                break;
            case CGETANSENSOR:
                n = mem[x];
                if (n < 1 || n > 6)
                    runtimeerr(wrong_ansensor_num, n, 0);
                memset(i2ccommand, '\0', I2CBUFFERSIZE);
                printf("i2cget -y 2 0x48 0x%x\n", 0x26 - n);
                snprintf(i2ccommand, I2CBUFFERSIZE, "i2cget -y 2 0x48 0x%x", 0x26 - n);
                mem[x] = rungetcommand(i2ccommand);
                break;
            case SLEEP:
                sleep(mem[x--]);
                break;
#endif
            case FUNCBEG:
                pc = mem[pc+1];
                break;
            case PRINT:
                auxprint(mem[x--], mem[pc++]);
                break;
            case PRINTID:
                i = mem[pc++];              // ссылка на identtab
                printident(identab[i+1]);   // ссылка на reprtab
                r = dspl(identab[i+3]);
                auxprint(mem[r],identab[i+2]);
                break;
            case GETID:
                i = mem[pc++];
                printf("\n");
                printident(identab[i+1]);
                r = dspl(identab[i+3]);
                auxget(&mem[r], identab[i+2]);
                break;
            case ABSIC:
                mem[x] = abs(mem[x]);
                break;
            case ABSC:
                rf = fabs(rf);
                memcpy(&mem[x], &rf, sizeof(num));
                break;
            case SQRTC:
                rf = sqrtf(rf);
                memcpy(&mem[x], &rf, sizeof(num));
                break;
            case EXPC:
                rf = expf(rf);
                memcpy(&mem[x], &rf, sizeof(num));
                break;
            case SINC:
                rf = sinf(rf);
                memcpy(&mem[x], &rf, sizeof(num));
                break;
            case COSC:
                rf = cosf(rf);
                memcpy(&mem[x], &rf, sizeof(num));
                break;
            case LOGC:
                rf = logf(rf);
                memcpy(&mem[x], &rf, sizeof(num));
                break;
            case LOG10C:
                rf = log10f(rf);
                memcpy(&mem[x], &rf, sizeof(num));
                break;
            case ASINC:
                rf = asinf(rf);
                memcpy(&mem[x], &rf, sizeof(num));
                break;
                
            case DECX:
                x--;
                break;
            case DEFARR:
                if (mem[x] <= 0)
                    runtimeerr(wrong_number_of_elems, 0, mem[x]);
                mem[dsp()] = x + 1;
                x += mem[x];
                break;
            case DEFARR2:
            {
                int oldx = x, m;
                int n = mem[x--], i;
                m = mem[x];
                if (m <= 0)
                    runtimeerr(wrong_number_of_elems, 0, m);
                if (n <= 0)
                    runtimeerr(wrong_number_of_elems, 0, n);
                mem[dsp()] = oldx;
                x += m;
                for (i=0; i<m; i++)
                {
                    mem[++x] = n;
                    mem[oldx+i] = x +1;
                    x += n;
                }
                break;
            }
            case LI:
                mem[++x] = mem[pc++];
                break;
            case LOAD:
                mem[++x] = mem[dsp()];
                break;
            case LAT:
                mem[x] = mem[mem[x]];
                break;
            case LA:
                mem[++x] = dsp();
                break;
            case CALL1:
                mem[l+1] = ++x;
                mem[x++] = l;
                mem[x++] = 0;    // следующая статика
                mem[x] = 0;      // pc в момент вызова
                break;
            case CALL2:
                i = mem[pc++];
                l = mem[l+1];
                entry = functions[i > 0 ? i : mem[l-i]];
                x = l + mem[entry+1] - 1;
                mem[l+2] = pc;
                pc = entry + 3;
                break;
            case _RETURN:
                pc = mem[l+2];
                r = l;
                l = mem[l];
                mem[l+1] = 0;
                mem[r] = mem[x];
                x = r ;
                break;
            case RETURNV:
                pc = mem[l+2];
                x = l-1;
                l = mem[l];
                mem[l+1] = 0;
                break;
            case NOP:
                ;
                break;
            case B:
            case STRING:
                pc = mem[pc];
                break;
            case BE0:
                pc = (mem[x--]) ?  pc + 1 : mem[pc];
                break;
            case BNE0:
                pc = (mem[x--]) ? mem[pc] : pc + 1;
                break;
            case SLICE:
                i = mem[x--];   // index
                r = mem[x];     // array
                if (i < 0 || i >= mem[r-1])
                    runtimeerr(index_out_of_range, i, mem[r-1]);
                mem[x] = r + i;
                break;
            case ASSARR:
                r = mem[dsp()];
                n = mem[pc++];
                if (mem[r-1] != n)
                    runtimeerr(wrong_arr_init, mem[r-1], n);
                r += n;
                for (i=n; i>0; i--)
                    mem[--r] = mem[x--];
                break;
            case ASSARR2:
            {
                int nr, m, n, i, j, rr;
                r = mem[dsp()];
                m = mem[r-1];
                n = mem[mem[r]-1];
                nr = mem[pc++];
                if (m*n != nr)
                    runtimeerr(wrong_arr_init, m*n, nr);
                r += m - 1;
                for (i=0; i<m; i++)
                {
                    rr = mem[r] + n;
                    for (j=n; j>0; j--)
                        mem[--rr] = mem[x--];
                    r--;
                }
                break;
            }
            case WIDEN:
                rf = (float)mem[x];
                memcpy(&mem[x], &rf, sizeof(num));
                break;
            case WIDEN1:
                rf = (float)mem[x-1];
                memcpy(&mem[x-1], &rf, sizeof(num));
                break;
            case DOUBLE:
                r = mem[x];
                mem[++x] = r;
                break;
            case ASS:
                mem[dsp()] = mem[x];
                break;
            case REMASS:
                r = mem[dsp()] %= mem[x];
                mem[x] = r;
                break;
            case SHLASS:
                r = mem[dsp()] <<= mem[x];
                mem[x] = r;
                break;
            case SHRASS:
                r = mem[dsp()] >>= mem[x];
                mem[x] = r;
                break;
            case ANDASS:
                r = mem[dsp()] &= mem[x];
                mem[x] = r;
                break;
            case EXORASS:
                r = mem[dsp()] ^= mem[x];
                mem[x] = r;
                break;
            case ORASS:
                r = mem[dsp()] |= mem[x];
                mem[x] = r;
                break;
            case PLUSASS:
                r = mem[dsp()] += mem[x];
                mem[x] = r;
                break;
            case MINUSASS:
                r = mem[dsp()] -= mem[x];
                mem[x] = r;
                break;
            case MULTASS:
                r = mem[dsp()] *= mem[x];
                mem[x] = r;
                break;
            case DIVASS:
                r = mem[dsp()] /= mem[x];
                mem[x] = r;
                break;
                
            case ASSV:
                mem[dsp()] = mem[x--];
                break;
            case REMASSV:
                mem[dsp()] %= mem[x--];
                break;
            case SHLASSV:
                mem[dsp()] <<= mem[x--];
                break;
            case SHRASSV:
                mem[dsp()] >>= mem[x--];
                break;
            case ANDASSV:
                mem[dsp()] &= mem[x--];
                break;
            case EXORASSV:
                mem[dsp()] ^= mem[x--];
                break;
            case ORASSV:
                mem[dsp()] |= mem[x--];
                break;
            case PLUSASSV:
                mem[dsp()] += mem[x--];
                break;
            case MINUSASSV:
                mem[dsp()] -= mem[x--];
                break;
            case MULTASSV:
                mem[dsp()] *= mem[x--];
                break;
            case DIVASSV:
                mem[dsp()] /= mem[x--];
                break;
                
            case ASSAT:
                r = mem[mem[x-1]] = mem[x];
                mem[--x] = r;
                break;
            case REMASSAT:
                r = mem[mem[x-1]] %= mem[x];
                mem[--x] = r;
                break;
            case SHLASSAT:
                r = mem[mem[x-1]] <<= mem[x];
                mem[--x] = r;
                break;
            case SHRASSAT:
                r = mem[mem[x-1]] >>= mem[x];
                mem[--x] = r;
                break;
            case ANDASSAT:
                r = mem[mem[x-1]] &= mem[x];
                mem[--x] = r;
                break;
            case EXORASSAT:
                r = mem[mem[x-1]] ^= mem[x];
                mem[--x] = r;
                break;
            case ORASSAT:
                r = mem[mem[x-1]] |= mem[x];
                mem[--x] = r;
                break;
            case PLUSASSAT:
                r = mem[mem[x-1]] += mem[x];
                mem[--x] = r;
                break;
            case MINUSASSAT:
                r = mem[mem[x-1]] -= mem[x];
                mem[--x] = r;
                break;
            case MULTASSAT:
                r = mem[mem[x-1]] *= mem[x];
                mem[--x] = r;
                break;
            case DIVASSAT:
                r = mem[mem[x-1]] /= mem[x];
                mem[--x] = r;
                break;
                
            case ASSATV:
                mem[mem[x-1]] = mem[x];
                x--;
                break;
            case REMASSATV:
                mem[mem[x-1]] %= mem[x];
                x--;
                break;
            case SHLASSATV:
                mem[mem[x-1]] <<= mem[x];
                x--;
                break;
            case SHRASSATV:
                mem[mem[x-1]] >>= mem[x];
                x--;
                break;
            case ANDASSATV:
                mem[mem[x-1]] &= mem[x];
                x--;
                break;
            case EXORASSATV:
                mem[mem[x-1]] ^= mem[x];
                x--;
                break;
            case ORASSATV:
                mem[mem[x-1]] |= mem[x];
                x--;
                break;
            case PLUSASSATV:
                mem[mem[x-1]] += mem[x];
                x--;
                break;
            case MINUSASSATV:
                mem[mem[x-1]] -= mem[x];
                x--;
                break;
            case MULTASSATV:
                mem[mem[x-1]] *= mem[x];
                x--;
                break;
            case DIVASSATV:
                mem[mem[x-1]] /= mem[x];
                x--;
                break;
                
            case LOGOR:
                mem[x-1] = mem[x-1] || mem[x];
                x--;
                break;
            case LOGAND:
                mem[x-1] = mem[x-1] && mem[x];
                x--;
                break;
            case LOR:
                mem[x-1] |= mem[x];
                x--;
                break;
            case LEXOR:
                mem[x-1] ^= mem[x];
                x--;
                break;
            case LAND:
                mem[x-1] &= mem[x];
                x--;
                break;
            case LSHR:
                mem[x-1] >>= mem[x];
                x--;
                break;
            case LSHL:
                mem[x-1] <<= mem[x];
                x--;
                break;
            case LREM:
                mem[x-1] %= mem[x];
                x--;
                break;
            case EQEQ:
                mem[x-1] = mem[x-1] == mem[x];
                x--;
                break;
            case NOTEQ:
                mem[x-1] = mem[x-1] != mem[x];
                x--;
                break;
            case LLT:
                mem[x-1] = mem[x-1] < mem[x];
                x--;
                break;
            case LGT:
                mem[x-1] = mem[x-1] > mem[x];
                x--;
                break;
            case LLE:
                mem[x-1] = mem[x-1] <= mem[x];
                x--;
                break;
            case LGE:
                mem[x-1] = mem[x-1] >= mem[x];
                x--;
                break;
            case LPLUS:
                mem[x-1] += mem[x];
                x--;
                break;
            case LMINUS:
                mem[x-1] -= mem[x];
                x--;
                break;
            case LMULT:
                mem[x-1] *= mem[x];
                x--;
                break;
            case LDIV:
                mem[x-1] /= mem[x];
                x--;
                break;
            case POSTINC:
                mem[++x] = mem[r=dsp()];
                mem[r]++;
                break;
            case POSTDEC:
                mem[++x] = mem[r=dsp()];
                mem[r]--;
                break;
            case INC:
                mem[++x] = ++mem[dsp()];
                break;
            case DEC:
                mem[++x] = --mem[dsp()];
                break;
            case POSTINCAT:
                mem[x] = mem[r=mem[x]];
                mem[r]++;
                break;
            case POSTDECAT:
                mem[x] = mem[r=mem[x]];
                mem[r]--;
                break;
            case INCAT:
                mem[x] = ++mem[mem[x]];
                break;
            case DECAT:
                mem[x] = --mem[mem[x]];
                break;
            case INCV:
            case POSTINCV:
                mem[dsp()]++;
                break;
            case DECV:
            case POSTDECV:
                mem[dsp()]--;
                break;
            case INCATV:
            case POSTINCATV:
                mem[mem[x--]]++;
                break;
            case DECATV:
            case POSTDECATV:
                mem[mem[x--]]--;
                break;

            case UNMINUS:
                mem[x] = -mem[x];
                break;
                
            case PLUSASSR:
                memcpy(&lf, &mem[i=dsp()], sizeof(num));
                lf += rf;
                memcpy(&mem[x], &lf, sizeof(num));
                memcpy(&mem[i], &lf, sizeof(num));
                break;
            case MINUSASSR:
                memcpy(&lf, &mem[i=dsp()], sizeof(num));
                lf -= rf;
                memcpy(&mem[x], &lf, sizeof(num));
                memcpy(&mem[i], &lf, sizeof(num));
                break;
            case MULTASSR:
                memcpy(&lf, &mem[i=dsp()], sizeof(num));
                lf *= rf;
                memcpy(&mem[x], &lf, sizeof(num));
                memcpy(&mem[i], &lf, sizeof(num));
                break;
            case DIVASSR:
                memcpy(&lf, &mem[i=dsp()], sizeof(num));
                lf /= rf;
                memcpy(&mem[x], &lf, sizeof(num));
                memcpy(&mem[i], &lf, sizeof(num));
                break;
                
            case PLUSASSATR:
                memcpy(&lf, &mem[i=mem[--x]], sizeof(num));
                lf += rf;
                memcpy(&mem[x], &lf, sizeof(num));
                memcpy(&mem[i], &lf, sizeof(num));
                break;
            case MINUSASSATR:
                memcpy(&lf, &mem[i=mem[--x]], sizeof(num));
                lf -= rf;
                memcpy(&mem[x], &lf, sizeof(num));
                memcpy(&mem[i], &lf, sizeof(num));
                break;
            case MULTASSATR:
                memcpy(&lf, &mem[i=mem[--x]], sizeof(num));
                lf *= rf;
                memcpy(&mem[x], &lf, sizeof(num));
                memcpy(&mem[i], &lf, sizeof(num));
                break;
            case DIVASSATR:
                memcpy(&lf, &mem[i=mem[--x]], sizeof(num));
                lf /= rf;
                memcpy(&mem[x], &lf, sizeof(num));
                memcpy(&mem[i], &lf, sizeof(num));
                break;
                
            case PLUSASSRV:
                memcpy(&lf, &mem[i=dsp()], sizeof(num));
                lf += rf;
                memcpy(&mem[i], &lf, sizeof(num));
                x--;
                break;
            case MINUSASSRV:
                memcpy(&lf, &mem[i=dsp()], sizeof(num));
                lf -= rf;
                memcpy(&mem[i], &lf, sizeof(num));
                x--;
                break;
            case MULTASSRV:
                memcpy(&lf, &mem[i=dsp()], sizeof(num));
                lf *= rf;
                memcpy(&mem[i], &lf, sizeof(num));
                x--;
                break;
            case DIVASSRV:
                memcpy(&lf, &mem[i=dsp()], sizeof(num));
                lf /= rf;
                memcpy(&mem[i], &lf, sizeof(num));
                x--;
                break;
                
            case PLUSASSATRV:
                memcpy(&lf, &mem[i=mem[--x]], sizeof(num));
                lf += rf;
                memcpy(&mem[i], &lf, sizeof(num));
                break;
            case MINUSASSATRV:
                memcpy(&lf, &mem[i=mem[--x]], sizeof(num));
                lf -= rf;
                memcpy(&mem[i], &lf, sizeof(num));
                break;
            case MULTASSATRV:
                memcpy(&lf, &mem[i=mem[--x]], sizeof(num));
                lf *= rf;
                memcpy(&mem[i], &lf, sizeof(num));
                break;
            case DIVASSATRV:
                memcpy(&lf, &mem[i=mem[--x]], sizeof(num));
                lf /= rf;
                memcpy(&mem[i], &lf, sizeof(num));
                break;
                
            case EQEQR:
                memcpy(&lf, &mem[--x], sizeof(num));
                mem[x] = lf == rf;
                break;
            case NOTEQR:
                memcpy(&lf, &mem[--x], sizeof(num));
                mem[x] = lf != rf;
                break;
            case LLTR:
                memcpy(&lf, &mem[--x], sizeof(num));
                mem[x] = lf < rf;
                break;
            case LGTR:
                memcpy(&lf, &mem[--x], sizeof(num));
                mem[x] = lf > rf;
                break;
            case LLER:
                memcpy(&lf, &mem[--x], sizeof(num));
                mem[x] = lf <= rf;
                break;
            case LGER:
                memcpy(&lf, &mem[--x], sizeof(num));
                mem[x] = lf >= rf;
                break;
            case LPLUSR:
                memcpy(&lf, &mem[--x], sizeof(num));
                lf += rf;
                memcpy(&mem[x], &lf, sizeof(num));
                break;
            case LMINUSR:
                memcpy(&lf, &mem[--x], sizeof(num));
                lf -= rf;
                memcpy(&mem[x], &lf, sizeof(num));
                break;
            case LMULTR:
                memcpy(&lf, &mem[--x], sizeof(num));
                lf *= rf;
                memcpy(&mem[x], &lf, sizeof(num));
                break;
            case LDIVR:
                memcpy(&lf, &mem[--x], sizeof(num));
                lf /= rf;
                memcpy(&mem[x], &lf, sizeof(num));
                break;
            case POSTINCR:
                memcpy(&rf, &mem[i=dsp()], sizeof(num));
                memcpy(&mem[++x], &rf, sizeof(num));
                ++rf;
                memcpy(&mem[i], &rf, sizeof(num));
                break;
            case POSTDECR:
                memcpy(&rf, &mem[i=dsp()], sizeof(num));
                memcpy(&mem[++x], &rf, sizeof(num));
                --rf;
                memcpy(&mem[i], &rf, sizeof(num));
                break;
            case INCR:
                memcpy(&rf, &mem[i=dsp()], sizeof(num));
                ++rf;
                memcpy(&mem[++x], &rf, sizeof(num));
                memcpy(&mem[i], &rf, sizeof(num));
                break;
            case DECR:
                memcpy(&rf, &mem[i=dsp()], sizeof(num));
                --rf;
                memcpy(&mem[++x], &rf, sizeof(num));
                memcpy(&mem[i], &rf, sizeof(num));
                break;
            case POSTINCATR:
                memcpy(&rf, &mem[i=mem[x]], sizeof(num));
                memcpy(&mem[++x], &rf, sizeof(num));
                ++rf;
                memcpy(&mem[i], &rf, sizeof(num));
                break;
            case POSTDECATR:
                memcpy(&rf, &mem[i=mem[x]], sizeof(num));
                memcpy(&mem[++x], &rf, sizeof(num));
                --rf;
                memcpy(&mem[i], &rf, sizeof(num));
                break;
            case INCATR:
                memcpy(&rf, &mem[i=mem[x]], sizeof(num));
                ++rf;
                memcpy(&mem[++x], &rf, sizeof(num));
                memcpy(&mem[i], &rf, sizeof(num));
                break;
            case DECATR:
                memcpy(&rf, &mem[i=mem[x]], sizeof(num));
                --rf;
                memcpy(&mem[++x], &rf, sizeof(num));
                memcpy(&mem[i], &rf, sizeof(num));
                break;
            case INCRV:
            case POSTINCRV:
                memcpy(&rf, &mem[i=dsp()], sizeof(num));
                ++rf;
                memcpy(&mem[i], &rf, sizeof(num));
                break;
            case DECRV:
            case POSTDECRV:
                memcpy(&rf, &mem[i=dsp()], sizeof(num));
                --rf;
                memcpy(&mem[i], &rf, sizeof(num));
                break;
            case INCATRV:
            case POSTINCATRV:
                memcpy(&rf, &mem[i=mem[x--]], sizeof(num));
                ++rf;
                memcpy(&mem[i], &rf, sizeof(num));
                break;
            case DECATRV:
            case POSTDECATRV:
                memcpy(&rf, &mem[i=mem[x--]], sizeof(num));
                --rf;
                memcpy(&mem[i], &rf, sizeof(num));
                break;

            case UNMINUSR:
                rf = -rf;
                memcpy(&mem[x], &rf, sizeof(num));
                break;
            case LNOT:
                mem[x] = ~ mem[x];
                break;
            case LOGNOT:
                mem[x] = ! mem[x];
                break;
                
            default:
                runtimeerr(wrong_kop, mem[pc-1], 0);
                break;
        }
    }
    
#ifdef ROBOT
    printf("111");
    system("i2cset -y 2 0x48 0x10 0 w");   // отключение силовых моторов
    system("i2cset -y 2 0x48 0x11 0 w");
    system("i2cset -y 2 0x48 0x12 0 w");
    system("i2cset -y 2 0x48 0x13 0 w");
    fclose(f1);
    fclose(f2);
#endif
    
    
}