#include "lib.h"
#include "symbol.h"
#include "type.h"
#include "trans.h"
#include "semantic_analysis_error.h"
#include "syntax.tab.h"

SymbolTable* table;
SymbolEntry* funcSymbolList;

SymbolTable* create_symbol_table(){
    SymbolTable *table = (SymbolTable*)calloc(1, sizeof(SymbolTable));
    for (int i = 0; i < HASH_SIZE; i++) {
        SymbolEntry* head = (SymbolEntry*)calloc(1, sizeof(SymbolEntry));
        table->buckets[i] = head;
    }
    for (int i = 0; i < MAX_DEPTH; i++) {
        table->scopes[i] = (SymbolEntry*)calloc(1, sizeof(SymbolEntry));
    }
    table->cur_depth = 0;
    funcSymbolList = (SymbolEntry*)calloc(1, sizeof(SymbolEntry));
    funcSymbolList->hash_next = NULL;
    return table;
}

static unsigned int hashmap(const char* name){
    unsigned int seed = 113;
    unsigned int c;
    while((c = *name++))
        seed = ((seed << 5) + seed) + c;
    return seed % SYMBOL_TABLE_SIZE;
}

void enter_scope(){
    table->cur_depth++;
}

void exit_scope() {
    /*
    In lab-3, it is necessary to query the type of variables. 
    If the symbols is destroyed after the function Program() ends, 
    it will be impossible to do so.
    So I temporarily disabled this function.
    */ 
    return;

    if (table->cur_depth == 0) {
        printf("Error: cannot exit scope\n");
        assert(0);
    }
    SymbolEntry* cur = table->scopes[table->cur_depth]->stack_next; 
    while (cur != NULL) {
        if(!cur->hash_prev)
            assert(0);
        cur->hash_prev->hash_next = cur->hash_next;
        if(cur->hash_next)
            cur->hash_next->hash_prev = cur->hash_prev;
        SymbolEntry* next = cur->stack_next;
        free(cur->name);
        free(cur);
        cur = next;
    }
    table->scopes[table->cur_depth]->stack_next = NULL; 
    table->cur_depth--;
}


int insert_symbol(const char* name, unsigned int line, Type type){
    if(type->kind == FUNCTION){
        return insert_function_symbol(name, line, type);
    }

    unsigned int bucket = hashmap(name);
    SymbolEntry *head = table->buckets[bucket];

    if (lookup_symbol(name) != NULL) {
            duplicate_handle(name, type, line);
            return 0; 
    }

    SymbolEntry *new_entry = calloc(1, sizeof(SymbolEntry));
    new_entry->name = (char*)malloc(strlen(name) + 1);
    strcpy(new_entry->name, name);
    new_entry->alias = (char*)malloc(MAX_CODE_LENGTH * sizeof(char));
    new_entry->alias = new_alias();
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
    return 1;
}

/*
    Why is this function set?
    Because other variables will be destroyed as they exit the scope, 
    but function names will not
*/
int insert_function_symbol(const char* name, unsigned int line, Type type){
    if(type->kind != FUNCTION){
        insert_symbol(name, line, type);
    }
    
    SymbolEntry* entry = funcSymbolList->hash_next;
    while(entry != NULL){
        if (strcmp(entry->name, name) == 0) {
            duplicate_handle(name, type, line);
            return 0; 
        }
        entry = entry->hash_next;
    }
    SymbolEntry* new_entry = calloc(1, sizeof(SymbolEntry));
    new_entry->depth = 0;
    new_entry->first_occur_line = line;
    new_entry->name = (char*)malloc(strlen(name) + 1);
    strcpy(new_entry->name, name);
    new_entry->type = type;
    new_entry->hash_next = funcSymbolList->hash_next;
    funcSymbolList->hash_next = new_entry;
    return 1;
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


SymbolEntry* lookup_symbol_with_a_type(const char *name, Kind kind) {
    unsigned int bucket;
    SymbolEntry *entry;
    if(kind == FUNCTION){
        entry = funcSymbolList->hash_next;
    }
    else{
        bucket = hashmap(name);
        entry = table->buckets[bucket]->hash_next;
    }
    while (entry != NULL) {
        if (strcmp(entry->name, name) == 0 && entry->type->kind == kind) {
            return entry; 
        }
        entry = entry->hash_next;
    }
    return NULL;
}


void FreeSymbolTable(){
    /*
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
    */
}

void duplicate_handle(const char* name,Type type, unsigned int line){
    if(type->kind == BASIC || type->kind == ARRAY)
        semErrOutput(DEFINE_VAR_MULTIPLY, line, name);
    else if(type->kind == STRUCTURE)
        semErrOutput(DEFINE_STRUCT_MULTIPLY, line, name);
    else if(type->kind == FUNCTION)
        semErrOutput(DEFINE_FUNC_MULTIPLY, line, name);
    else
        assert(0);
}



