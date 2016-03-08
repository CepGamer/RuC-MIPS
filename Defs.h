//
//  Defs.h
//  RuC
//
//  Created by Andrey Terekhov on 03/06/14.
//  Copyright (c) 2014 Andrey Terekhov. All rights reserved.
//

#ifndef RuC_Defs_h
#define RuC_Defs_h

#define MAXREPRTAB 10000
#define MAXIDENTAB 10000
#define MAXMEMSIZE 10000
#define MAXTREESIZE 10000
#define MAXMODSIZE 10000
#define FUNCSIZE   1000
#define SOURCESIZE 10000
#define LINESSIZE  3000
#define MAXSTRINGL 128

// Лексемы операций языка С

#define ASS        1
#define REMASS     2
#define SHLASS     3
#define SHRASS     4
#define ANDASS     5
#define EXORASS    6
#define ORASS      7

#define PLUSASS    8
#define MINUSASS   9
#define MULTASS   10
#define DIVASS    11

#define ASSAT      12   // эти 22 операции с присваиванием оставляют значение на стеке
#define REMASSAT   13
#define SHLASSAT   14
#define SHRASSAT   15
#define ANDASSAT   16
#define EXORASSAT  17
#define ORASSAT    18

#define PLUSASSAT  19
#define MINUSASSAT 20
#define MULTASSAT  21
#define DIVASSAT   22

#define ASSV       201
#define REMASSV    202
#define SHLASSV    203
#define SHRASSV    204
#define ANDASSV    205
#define EXORASSV   206
#define ORASSV     207

#define PLUSASSV   208
#define MINUSASSV  209
#define MULTASSV   210
#define DIVASSV    211

#define ASSATV     212   // а эти 22 операции с присваиванием не оставляют значение на стеке
#define REMASSATV  213
#define SHLASSATV  214
#define SHRASSATV  215
#define ANDASSATV  216
#define EXORASSATV 217
#define ORASSATV   218

#define PLUSASSATV 219
#define MINUSASSATV 220
#define MULTASSATV 221
#define DIVASSATV  222

#define LREM      23
#define LSHL      24
#define LSHR      25
#define LAND      26
#define LEXOR     27
#define LOR       28
#define LOGAND    29
#define LOGOR     30

#define EQEQ      31
#define NOTEQ     32
#define LLT       33
#define LGT       34
#define LLE       35
#define LGE       36
#define LPLUS     37
#define LMINUS    38
#define LMULT     39    // у этих 27 команд есть такие же с плавающей зпт
#define LDIV      40

#define POSTINC   41
#define POSTDEC   42
#define INC       43
#define DEC       44
#define POSTINCAT 45
#define POSTDECAT 46
#define INCAT     47
#define DECAT     48
#define POSTINCV   241
#define POSTDECV   242
#define INCV       243
#define DECV       244
#define POSTINCATV 245
#define POSTDECATV 246
#define INCATV     247
#define DECATV     248


#define UNMINUS   49

#define ANDADDR   50
#define MULTADDR  51
#define LNOT      52
#define LOGNOT    53


#define PLUSASSR    58
#define MINUSASSR   59
#define MULTASSR    60
#define DIVASSR     61

#define PLUSASSATR  69
#define MINUSASSATR 70
#define MULTASSATR  71
#define DIVASSATR   72

#define PLUSASSRV    258
#define MINUSASSRV   259
#define MULTASSRV    260
#define DIVASSRV     261

#define PLUSASSATRV  269
#define MINUSASSATRV 270
#define MULTASSATRV  271
#define DIVASSATRV   272

#define EQEQR     81
#define NOTEQR    82
#define LLTR      83
#define LGTR      84
#define LLER      85
#define LGER      86
#define LPLUSR    87
#define LMINUSR   88
#define LMULTR    89
#define LDIVR     90

#define POSTINCR  91
#define POSTDECR  92
#define INCR      93
#define DECR      94
#define POSTINCATR 95
#define POSTDECATR 96
#define INCATR     97
#define DECATR     98
#define POSTINCRV  291
#define POSTDECRV  292
#define INCRV      293
#define DECRV      294
#define POSTINCATRV 295
#define POSTDECATRV 296
#define INCATRV     297
#define DECATRV     298


#define UNMINUSR   99

// Коды операций виртуальной машины, кроме операций C

#define DECX    151

#define NOP     153
#define DEFARR  154
#define LI      155
#define LOAD    156
#define LAT     157       // это L@
#define STOP    158
#define FUNCBEG 160       // обозначает начало кода функции
#define LA      161
#define CALL1   162
#define CALL2   163
#define _RETURN 164
#define RETURNV 165
#define B       166
#define BE0     167
#define BNE0    168
#define SLICE   169
#define ASSARR  170
#define WIDEN   171
#define WIDEN1  172
#define DOUBLE  173
#define DEFARR2 174
#define ASSARR2 175

// Коды операций стандартных функций

#define ABSIC     423
#define GETDIGSENSORC 424
#define GETANSENSORC 425
#define ABSC      426
#define SQRTC     427
#define EXPC      428
#define SINC      429
#define COSC      430
#define LOGC      431
#define LOG10C    432
#define ASINC     433


// Лексемы

#define COMMA     100
#define QUEST     101
#define COLON     102
#define LEFTBR    103
#define RIGHTBR   104
#define LEFTSQBR  105
#define RIGHTSQBR 106
#define STRING    107
#define NUMBER    108
#define IDENT     109
#define BEGIN     115
#define END       116
#define SEMICOLON 117
#define LAPOST    118
#define LQUOTE    119
#define LEOF      120
#define DOT       121
#define ARROW     122

// Ответы

#define VAL       110
#define ADDR      111
#define F         112

// Ключевые слова

#define LMAIN   0
#define LINT   -1
#define LCHAR  -2
#define LFLOAT -3
#define LVOID  -4

#define ROWOFINT   -5
#define ROWOFCHAR  -6
#define ROWOFFLOAT -7
#define ROWROWOFINT   -15
#define ROWROWOFCHAR  -16
#define ROWROWOFFLOAT -17

#define FUNCINT    -9
#define FUNCCHAR  -10
#define FUNCFLOAT -11
#define FUNCVOID  -12

#define LBREAK -5
#define LCASE  -6
#define LCONTINUE -7
#define LDEFAULT  -8
#define LDO    -9
#define LELSE  -10
#define LENUM  -11
#define LFOR   -12
#define LGOTO  -13
#define LIF    -14
#define LRETURN -15
#define LSIZEOF -16
#define LSWITCH -17
#define LWHILE  -18
#define PRINTID -19
#define PRINT   -20
#define GETID   -21
#define SETMOTOR  -22
#define SLEEP     -23     // до этого места операторы, а затем функции
#define GETDIGSENSOR -24
#define GETANSENSOR -25
#define ABS      -26
#define SQRT     -27
#define EXP      -28
#define SIN      -29
#define COS      -30
#define LOG      -31
#define LOG10    -32
#define ASIN     -33


// Узлы дерева
#define TIdent      -300
#define TConst      -301
#define TString     -302
#define TSliceident -303
#define TSlice      -304
#define TIdenttoval -305
#define TAddrtoval  -306
#define TCall1      -307
#define TCall2      -308

#define TFuncdef    -309
#define TDeclid     -310
#define TInit       -311
#define TExprend    -312
#define TCondexpr   -313
#define TBegin      -314
#define TEnd        -315
#define TIf         -316
#define TWhile      -317
#define TDo         -318
#define TFor        -319
#define TSwitch     -320
#define TCase       -321
#define TDefault    -322
#define TBreak      -323
#define TContinue   -324
#define TReturn     -325
#define TReturnval  -326
#define TGoto       -327
#define TLabel      -328
#define TPrint      -329
#define TPrintid    -330
#define TGetid      -331

// Коды ошибок

#define after_type_must_be_ident           201
#define wait_right_sq_br                   202
#define only_functions_may_have_type_VOID  203
#define decl_and_def_have_diff_type        204
#define decl_must_start_from_ident_or_decl 205
#define no_comma_in_param_list             206
#define wrong_param_list                   207
#define no_comma_in_type_list              208
#define wrong_type_list                    209
#define func_def_must_be_first             210
#define func_def_must_have_param_list      211
#define def_must_end_with_semicomma        212
#define func_and_protot_have_dif_num_params 213
#define param_types_are_dif                214
#define wait_ident_after_comma_in_decl     215
#define wait_rightbr_in_call               216
#define func_decl_req_params               217
#define wait_while_in_do_stmt              218
#define no_semicolon_after_stmt            219
#define cond_must_be_in_brkts              220
#define repeated_decl                      221
#define arr_init_must_start_from_BEGIN     222
#define no_comma_in_init_list              223
#define ident_is_not_declared              224
#define no_rightsqbr_in_slice              225
#define void_in_expr                       226
#define index_must_be_int                  227
#define slice_not_from_array               228
#define call_not_from_function             229
#define no_comma_in_act_params             230
#define float_instead_int                  231
#define wrong_number_of_params             232
#define wait_rightbr_in_primary            233
#define unassignable_inc                   234
#define wrong_addr                         235
#define no_colon_in_cond_expr              236
#define not_assignable                     237
#define func_not_in_call                   238
#define no_colon_in_case                   239
#define case_after_default                 240
#define no_ident_after_goto                241
#define no_leftbr_in_for                   242
#define no_semicolon_in_for                243
#define no_rightbr_in_for                  244
#define int_op_for_float                   245
#define assmnt_float_to_int                246
#define more_than_1_main                   247
#define no_main_in_program                 248
#define no_leftbr_in_printid               249
#define no_rightbr_in_printid              250
#define no_ident_in_printid                251
#define float_in_switch                    252
#define init_int_by_float                  253
#define must_be_digit_after_dot            254
#define no_leftbr_in_setmotor              255
#define no_rightbr_in_setmotor             256
#define no_comma_in_setmotor               257
#define param_setmotor_not_int             258
#define no_leftbr_in_sleep                 259
#define no_rightbr_in_sleep                260
#define no_leftbr_in_stand_func            261
#define no_rightbr_in_stand_func           262
#define bad_param_in_stand_func            263
#define no_ret_in_func                     264
#define bad_type_in_ret                    265
#define notvoidret_in_void_func            266
#define bad_escape_sym                     267
#define no_right_apost                     268
#define decl_after_strmt                   269
#define too_long_string                    270
#define no_ident_after_aster               271
#define aster_before_func                  272
#define aster_not_for_pointer              273
#define aster_with_row                     274
#define float_in_condition                 275
#define wrong_fun_as_param                 276
#define no_right_br_in_paramfun            277
#define no_ident_in_paramfun               278
#define par_type_void_with_nofun           279
#define ident_in_declarator                280
#define array_before_func                  281
#define wait_definition                    282
#define wait_declarator                    283
#define two_idents_for_1_declarer          284
#define function_has_no_body               285
#define declarator_in_call                 286
#define diff_formal_param_type_and_actual  287
#define case_or_default_not_in_switch      288
#define break_not_in_loop_or_switch        289
#define continue_not_in_loop               290
#define not_primary                        291
#define wrong_operand                      292
#define must_be_digit_after_exp            293
#define label_not_declared                 294
#define repeated_label                     295


#endif
