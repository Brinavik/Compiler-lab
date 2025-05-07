#ifndef _TRANS_MIPS_H_
#define _TRANS_MIPS_H_

#include "lib.h"
#include "trans.h"

#define TRANS_MIPS_DEBUG 1

typedef enum {REGISTER_FREE, REGISTER_OCCUPIED} RegState;
typedef struct Reg {
    char name[6];
    RegState state;
    char* value;
} Reg;
    /*
        Registers.
    */
Reg regs[32];

// Definition of the variables in a stack-frame.
typedef struct FrameVar_* FrameVar;
typedef struct FrameVar_{
    char name[MAX_CODE_LENGTH];
    unsigned int reg;
    int offset;
    FrameVar next;
} FrameVar_;


extern int local_offset;   // The offset of the variables in a stack-frame.
extern FrameVar frameVarHead;

void init_regs();
unsigned int get_free_reg();
FrameVar find_frame_var(Op op);
void insert_frame_var(FrameVar temp);
void insert_frame_var_for_op(Op op);

extern Codelist codelist;
void Trans_MIPS(char* file);
void Trans_MIPS_SingleLineCode(Code* code, FILE* file);

void Trans_MIPS_read(Code* code);
void Trans_MIPS_write(Code* code);
void Trans_MIPS_label(Code* code);
void Trans_MIPS_add(Code* code);
void Trans_MIPS_sub(Code* code);
void Trans_MIPS_mul(Code* code);
void Trans_MIPS_div(Code* code);
void Trans_MIPS_assignop(Code* code);
void Trans_MIPS_arg_call(Code* code);
void Trans_MIPS_goto(Code* code);
void Trans_MIPS_return(Code* code);
void Trans_MIPS_ifgoto(Code* code);
void Trans_MIPS_function(Code* code, FILE* file);

#endif