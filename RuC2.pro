TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.c \
    mips_compile.c \
    ext_decl.c \
    codegen.c \
    codes.c \
    import.c \
    scaner.c \
    error.c

HEADERS += \
    global_vars.h \
    mips_defs.h \
    Defs.h

