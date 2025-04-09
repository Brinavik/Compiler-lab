#ifndef SYMBOL_H
#define SYMBOL_H

#include <stdlib.h>
#include "type.h"

#define SYMBOL_TABLE_SIZE 97
#define HASH_SIZE SYMBOL_TABLE_SIZE
#define MAX_DEPTH 16


typedef struct SymbolEntry {
    char *name;          
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
static SymbolTable* table;

SymbolTable* create_symbol_table();
void enter_scope();
void exit_scope();
void insert_symbol(const char* name,unsigned int line, Type type);
SymbolEntry* lookup_symbol(const char *name);
SymbolEntry* lookup_symbol_with_a_type(const char *name, Type type);
void duplicate_handle(const char* name,Type type, unsigned int line);
void FreeSymbolTable();

#endif