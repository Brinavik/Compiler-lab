#ifndef SYMBOL_H
#define SYMBOL_H

#include <stdlib.h>
#include "type.h"
#include "trans.h"

#define SYMBOL_TABLE_SIZE 97
#define HASH_SIZE SYMBOL_TABLE_SIZE
#define MAX_DEPTH 32


typedef struct SymbolEntry {
    char *name; 
    /*
        alias is used in intermediate code generation.
        Create an alias for each variable,
        and reference it in the intermediate code generation.
    */         
    char* alias;
    // 新增字段：1表示函数参数，0表示普通变量
    unsigned int is_param;   
    Type type; 
    unsigned int first_occur_line;
    unsigned int depth;
    struct SymbolEntry *hash_next;
    struct SymbolEntry *hash_prev;
    struct SymbolEntry *stack_next;
} SymbolEntry;

typedef struct SymbolTable{
    unsigned int cur_depth;
    SymbolEntry* buckets[HASH_SIZE];
    SymbolEntry* scopes[MAX_DEPTH];
}SymbolTable;

extern SymbolTable* table;
extern SymbolEntry* funcSymbolList;

SymbolTable* create_symbol_table();
void enter_scope();
void exit_scope();
int insert_symbol(const char* name, unsigned int line, Type type);
/*
    Why is this function set?
    Because other variables will be destroyed as they exit the scope, 
    but function names will not
*/
int insert_function_symbol(const char* name, unsigned int line, Type type);
SymbolEntry* lookup_symbol(const char *name);
SymbolEntry* lookup_symbol_with_a_type(const char *name, Kind kind);
void duplicate_handle(const char* name,Type type, unsigned int line);
void FreeSymbolTable();

#endif
