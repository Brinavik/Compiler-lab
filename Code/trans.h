#ifndef _TRANS_H_
#define _TRANS_H_

#include "node.h"
#include "type.h"
#include "semantic.h"

#define MAX_CODE_LENGTH 35

// 定义全局常量，控制是否打印调试信息
#define TRANS_PRINT_DEBUG 0
// 新增数组支持开关
#define TRANS_ARRAY_SUPPORT 1  
// 输出控制符，为0输出中间代码，为1输出汇编指令
#define TRANS_MIPS_OUTPUT 1

typedef enum{
        OP_EMPTY, OP_CONST, OP_ADDR, OP_DEREF, OP_TEMP, OP_LABEL
} OpType;

typedef struct Op{
    char* name;
    OpType optype;
} Op;

typedef enum {CODE_UNKNOWN, CODE_READ, CODE_WRITE, CODE_LABEL, CODE_ADD, 
            CODE_SUB, CODE_MUL, CODE_DIV, CODE_SIMPLE_ASSIGNOP, CODE_ARG,
            CODE_CALL, CODE_GOTO, CODE_RETURN, CODE_IFGOTO, CODE_FUNCTION,
            CODE_PARAM, CODE_DEC, CODE_HANDLED, 
            CODE_ASSIGN_ADDR, CODE_ASSIGN_DEREF} CodeType;

typedef struct Code{
    char str[MAX_CODE_LENGTH];
    struct Code* next;
/*
    CodeType and Op are used in lab4.

    Referring to the middle code table in the lab3 lecture notes, 
    in the intermediate code, from left to right, they are op0, op1, and op2.
    If there are less than 3 variables in the middle code, then the additional op is empty.
*/
    CodeType codetype;
    Op ops[3];
}Code;

typedef struct Arg{
    char* name;
    struct Arg* next;
}Arg;

typedef struct Codelist{
    Code* head;
    Code* tail;
}Codelist;
extern Codelist codelist;

char* new_temp();
char* new_label();
char* new_alias();

void InitBasicComponents();
void codelist_append(Code* code);
void print_intermediate_code();
void Trans_Program();
void Trans_ExtDefList(Node* node);
void Trans_ExtDef(Node* node);
void Trans_FunDec(Node* node);
void Trans_VarList(Node* node);
void Trans_ParamDec(Node* node);
void Trans_CompSt(Node* node);
void Trans_StmtList(Node* node);
void Trans_Stmt(Node* node);
void Trans_ExtDecList(Node* node);
void Trans_Specifier(Node* node);
void Trans_StructSpecifier(Node* node);
void Trans_OptTag(Node* node);
void Trans_Tag(Node* node);
void Trans_DefList(Node* node);
void Trans_Def(Node* node);
void Trans_DecList(Node* node);
void Trans_Dec(Node* node);
void Trans_Args(Node* node, Arg** arg_list_head, Arg** arg_list_tail);
void Trans_VarDec(Node* node);
void Trans_Exp(Node* node, char* place);

#if TRANS_ARRAY_SUPPORT
void Trans_Exp_Addr(Node* node, char* place);
// 新增辅助函数：计算数组总大小（字节数）
int get_array_size(Type type);
#endif

// insert an "op" in a code.
static void insert_op(Code* code, char* opname, unsigned int opnum);
static void insert_ops(Code* code, char* name0, char* name1, char* name2);
static int is_valid_identifier_char(char c);
static int is_relop_char(char c);
void print_code_codetype(Code* code);
char* concatenate_char_with_string(char c, const char* name);
char* my_strdup(const char* s); 
#endif