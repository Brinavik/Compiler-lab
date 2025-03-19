%{
    #include <stdio.h>
    #include <errno.h>
    #include <stdlib.h>
    #include <string.h>
    #include "syntax.tab.h"
    #include "node.h"
    extern int has_error; 
//
    #define SYMBOL_TABLE_SIZE 97
    typedef enum { TYPE_INT, TYPE_FLOAT, TYPE_UNKNOWN } SymbolType;
    typedef struct SymbolEntry {
        char *name;          
        SymbolType type;    
        unsigned int scope_level;     
        unsigned int first_occur_line;
        struct SymbolEntry *next; 
        // maybe more ... 
    } SymbolEntry;
    typedef struct SymbolTable{
        SymbolEntry* buckets[SYMBOL_TABLE_SIZE];
        // maybe more ...
    }SymbolTable;
    SymbolTable* table;

    SymbolTable* create_symbol_table(){
        SymbolTable *table = malloc(sizeof(SymbolTable));
        memset(table->buckets, 0, sizeof(table->buckets));
        return table;
    }
    
    unsigned int hashmap(const char* name){
        unsigned int seed = 113;
        unsigned int c;
        while((c = *name++))
            seed = ((seed << 5) + seed) + c;
        return seed % SYMBOL_TABLE_SIZE;
    }
    
    void insert_symbol(SymbolTable *table, const char* name,unsigned int line, SymbolType type){   // maybe more paras...
        unsigned int bucket = hashmap(name);
        SymbolEntry *entry = table->buckets[bucket];
        while(entry != NULL){
            if(strcmp(entry->name, name) == 0)
        // maybe more ...
                return;
        }
        SymbolEntry *new_entry = malloc(sizeof(SymbolEntry));
        new_entry->name = strdup(name);
        new_entry->first_occur_line = line;
        new_entry->type = type;
        new_entry->next = table->buckets[bucket];
        table->buckets[bucket] = new_entry;
    }
    
    SymbolEntry* lookup_symbol(SymbolTable *table, const char *name) {
        unsigned int bucket = hashmap(name);
        SymbolEntry *entry = table->buckets[bucket];
        while (entry != NULL) {
            if (strcmp(entry->name, name) == 0) {
        // maybe more ... 
                return entry; 
            }
            entry = entry->next;
        }
        return NULL;
    }
    
    void FreeSymbolTable(SymbolTable* table){
        for(int i = 0; i < SYMBOL_TABLE_SIZE; i++){
            SymbolEntry* entry = table->buckets[i];
            while(entry != NULL){
                SymbolEntry* tmp = entry;
                entry = entry->next;
                free(tmp->name);
                free(tmp);
            } 
        }
        free(table);
    }

    // position
    int yycolumn = 1;
    #define YY_USER_ACTION \
    yylloc.first_line = yylloc.last_line = yylineno; \
    yylloc.first_column = yycolumn; \
    yylloc.last_column = yycolumn + yyleng - 1; \
    yycolumn += yyleng;
%}

%option yylineno

LETTER [a-zA-Z]
SEMI ;
COMMA ,
ASSIGNOP =
RELOP (>|<|>=|<=|==|!=)
PLUS \+
MINUS -
STAR \*
DIV \/
AND &&
OR \|\|
NOT !
TYPE (int|float)
LP \(
RP \)
LB \[
RB \]
LC \{
RC \}
DOT \.
STRUCT struct
RETURN return
IF if
ELSE else
WHILE while
INT ([1-9][0-9]*|0)
FLOAT (0[.][0-9]+|[1-9][0-9]*\.[0-9]+)
ID [_a-zA-Z][_a-zA-Z0-9]*
SINGLE_LINE_COMMENT \/\/.*
MULTI_LINE_COMMENT \/\*([^*]|\*+[^*/])*\*+\/
UNKNOWN_CHAR .

%%

{SEMI}     {yylval.node = create_node(1, "SEMI"    , ""    ); return SEMI;}
{COMMA}    {yylval.node = create_node(1, "COMMA"   , ""    ); return COMMA;}
{ASSIGNOP} {yylval.node = create_node(1, "ASSIGNOP", ""    ); return ASSIGNOP;}
{RELOP}    {yylval.node = create_node(1, "RELOP"   , ""    ); return RELOP;}
{PLUS}     {yylval.node = create_node(1, "PLUS"    , ""    ); return PLUS;}
{MINUS}    {yylval.node = create_node(1, "MINUS"   , ""    ); return MINUS;}
{STAR}     {yylval.node = create_node(1, "STAR"    , ""    ); return STAR;}
{DIV}      {yylval.node = create_node(1, "DIV"     , ""    ); return DIV;}
{AND}      {yylval.node = create_node(1, "AND"     , ""    ); return AND;}
{OR}       {yylval.node = create_node(1, "OR"      , ""    ); return OR;}
{NOT}      {yylval.node = create_node(1, "NOT"     , ""    ); return NOT;}
{TYPE}     {yylval.node = create_node(1, "TYPE"    , yytext); return TYPE;}
{DOT}      {yylval.node = create_node(1, "DOT"     , ""    ); return DOT;}
{LP}       {yylval.node = create_node(1, "LP"      , ""    ); return LP;}
{RP}       {yylval.node = create_node(1, "RP"      , ""    ); return RP;}
{LB}       {yylval.node = create_node(1, "LB"      , ""    ); return LB;}
{RB}       {yylval.node = create_node(1, "RB"      , ""    ); return RB;}
{LC}       {yylval.node = create_node(1, "LC"      , ""    ); return LC;}
{RC}       {yylval.node = create_node(1, "RC"      , ""    ); return RC;}
{STRUCT}   {yylval.node = create_node(1, "STRUCT"  , ""    ); return STRUCT;}
{RETURN}   {yylval.node = create_node(1, "RETURN"  , ""    ); return RETURN;}
{IF}       {yylval.node = create_node(1, "IF"      , ""    ); return IF;}
{ELSE}     {yylval.node = create_node(1, "ELSE"    , ""    ); return ELSE;}
{WHILE}    {yylval.node = create_node(1, "WHILE"   , ""    ); return WHILE;}
{FLOAT}    {yylval.node = create_node(1, "FLOAT"   , yytext); return FLOAT;}
{INT}      {yylval.node = create_node(1, "INT"     , yytext); return INT;}
{ID}       {yylval.node = create_node(1, "ID"      , yytext); return ID;}
{SINGLE_LINE_COMMENT} {}
{MULTI_LINE_COMMENT} {}
\n  {yycolumn = 1;}
[ \t\r]+ {}
{UNKNOWN_CHAR} {has_error = 1; printf("Error type A at Line %d: Mysterious characters \'%s\'\n", yylineno, yytext);}
%%

