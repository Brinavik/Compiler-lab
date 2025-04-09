#include <stdlib.h>
#include <string.h>
// #include "syntax.tab.h"
#include "type.h" 
#include "symbol.h"
#include "semantic_analysis_error.h"

SymbolTable* create_symbol_table(){
    SymbolTable *table = (SymbolTable*)malloc(sizeof(SymbolTable));
    for (int i = 0; i < HASH_SIZE; i++) {
        SymbolEntry* head = (SymbolEntry*)malloc(sizeof(SymbolEntry));
        table->buckets[i] = head;
    }

    for (int i = 0; i < MAX_DEPTH; i++) {
        table->scopes[i] = (SymbolEntry*)malloc(sizeof(SymbolEntry));
    }
    table->cur_depth = 0;
    return table;
}

void enter_scope(){
    if(table->cur_depth + 1 > MAX_DEPTH){
        printf("Error: SymbolTable depth overflow, enter_scope() failed !\n");
        assert(0);
    }
    table->cur_depth++;
}

void exit_scope(){
    SymbolEntry* cur = table->scopes[table->cur_depth]->stack_next;
    while (cur != NULL) {
        cur->hash_prev->hash_next = cur->hash_next;
        if (cur->hash_next) cur->hash_next->hash_prev = cur->hash_prev;
        SymbolEntry *next = cur->stack_next;
        free(cur->name);
        free(cur);
        cur = next;
    }
    table->cur_depth--;
}

static unsigned int hashmap(const char* name){
    unsigned int seed = 113;
    unsigned int c;
    while((c = *name++))
        seed = ((seed << 5) + seed) + c;
    return seed % SYMBOL_TABLE_SIZE;
}

void insert_symbol(const char* name, unsigned int line, Type type){
    unsigned int bucket = hashmap(name);
    SymbolEntry *head = table->buckets[bucket];

    SymbolEntry* duplicate_entry = NULL;
    if((duplicate_entry = lookup_symbol(name)) != NULL && duplicate_entry->depth == table->cur_depth) {
        duplicate_handle(name, type, line);
        return;
    }

    SymbolEntry *new_entry = malloc(sizeof(SymbolEntry));
    new_entry->name = strdup(name);
    new_entry->first_occur_line = line;
    new_entry->type = type;
    new_entry->depth = table->cur_depth;

    new_entry->hash_prev = head;
    new_entry->hash_next = head->hash_next;
    head->hash_next = new_entry;
    if (new_entry->hash_next) new_entry->hash_next->hash_prev = new_entry;

    SymbolEntry* scope_head = table->scopes[table->cur_depth];
    new_entry->stack_next = scope_head->stack_next;
    scope_head->stack_next = new_entry;

}

SymbolEntry* lookup_symbol(const char *name) {
    unsigned int bucket = hashmap(name);
    SymbolEntry *entry = table->buckets[bucket]->hash_next;
    while (entry != NULL) {
        if (strcmp(entry->name, name) == 0) {
            return entry; 
        }
        entry = entry->hash_next;
    }
    return NULL;
}


SymbolEntry* lookup_symbol_with_a_type(const char *name, Type type) {
    unsigned int bucket = hashmap(name);
    SymbolEntry *entry = table->buckets[bucket]->hash_next;
    while (entry != NULL) {
        if (strcmp(entry->name, name) == 0 && entry->type == type) {
            return entry; 
        }
        entry = entry->hash_next;
    }
    return NULL;
}



void FreeSymbolTable(){
    for(int i = 0; i < HASH_SIZE; i++){
        SymbolEntry* entry = table->buckets[i];
        while(entry != NULL){
            SymbolEntry* tmp = entry;
            entry = entry->hash_next;
            free(tmp->name);
            free(tmp);
        } 
    }
    free(table);
}

void duplicate_handle(const char* name,Type type, unsigned int line){
    if(type->kind == BASIC || type->kind == ARRAY)
        semErrOutput(DEFINE_VAR_MULTIPLY, line);
    else if(type->kind == STRUCTURE)
        semErrOutput(DEFINE_STRUCT_MULTIPLY, line);
    else if(type->kind == FUNCTION)
        semErrOutput(DEFINE_FUNC_MULTIPLY, line);
    else
        assert(0);
}