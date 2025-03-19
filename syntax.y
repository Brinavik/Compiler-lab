%{
    #include <stdio.h>
    #include <assert.h>
    #include <string.h>
    #include <stdlib.h>
    #include <stdarg.h>
    #include "node.h"
    #include "lex.yy.c"

void yyerror(char* msg, ...);
Node* create_node(NodeType type, char* name, char* attr) {
    Node* node = (Node*)malloc(sizeof(struct Node));
    node->type = type;
    node->name = strdup(name);
    node->attr = strdup(attr);
    return node;
}

Node* create_syntax(NodeType type, char* name,unsigned int line) {
    sprintf(buffer, "%d", line); 
    return create_node(type, name, buffer);
}

void add_node(unsigned int num, Node* parent, ...) {
    if(num == 0)    return;
    va_list args;
    va_start(args, parent);
    for (int i = 0; i < num; i++) {
        Node* child = va_arg(args, Node*);
        parent->child[parent->num] = child;
        parent->num++;
    }
    va_end(args);
}

void preorder(Node* node, int depth) {
    if (node == NULL) return ;
    for (int i = 0; i < depth; i++) printf("  ");
    depth++;
    if (node->type == SYNTAX_NODE) {
        printf("%s (%s)\n", node->name, node->attr);
    } else if (strcmp(node->name, "ID") == 0 || strcmp(node->name, "TYPE") == 0 || 
              strcmp(node->name, "INT") == 0 || strcmp(node->name, "FLOAT") == 0) {
        printf("%s: %s\n", node->name, node->attr);    
    } else {
        printf("%s\n", node->name);
    }

    for (int i = 0; i < node->num; i++) {
        preorder(node->child[i], depth);
    }
}

void display() {
    preorder(root, 0);
}
%}




%union
{
    struct Node* node;
}

%token <node> LETTER
%token <node> SEMI COMMA
%token <node> TYPE
%token <node> STRUCT
%token <node> IF WHILE RETURN
%token <node> INT FLOAT
%token <node> ID
%token <node> ASSIGNOP
%token <node> OR
%token <node> AND
%token <node> RELOP
%token <node> PLUS MINUS
%token <node> STAR DIV
%token <node> NOT
%token <node> LP RP LB RB DO LC RC DOT
%token <node> ELSE

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%right ASSIGNOP
%left OR
%left AND
%left RELOP
%left PLUS MINUS
%left STAR DIV
%right NOT
%left LP RP LB RB DO LC RC DOT

%left error 

%type <node> Program ExtDefList ExtDef Specifier ExtDecList FunDec CompSt VarDec StructSpecifier OptTag DefList Tag StmtList Stmt Exp Args DecList Dec Def VarList ParamDec

%%
// High-Level Definitions
Program : ExtDefList {$$ = create_syntax(0, "Program", @$.first_line); add_node(1, $$, $1); root = $$;}
    ;

ExtDefList : ExtDef ExtDefList {$$ = create_syntax(0, "ExtDefList", @$.first_line); add_node(2, $$, $1, $2);}
    | %empty {$$ = NULL;}
    ;

ExtDef : Specifier ExtDecList SEMI {$$ = create_syntax(0, "ExtDef", @$.first_line); add_node(3, $$, $1, $2, $3);}
    | Specifier SEMI {$$ = create_syntax(0, "ExtDef", @$.first_line); add_node(2, $$, $1, $2);}
    | Specifier FunDec CompSt {$$ = create_syntax(0, "ExtDef", @$.first_line); add_node(3, $$, $1, $2, $3);}
    | error SEMI {yyerror("Wrong ExtDef", @1.first_line); yyerrok; yyclearin; }
    | Specifier error {yyerror("Missing ;", @1.first_line); yyerrok; yyclearin; }
    ;

ExtDecList : VarDec {$$ = create_syntax(0, "ExtDecList", @$.first_line); add_node(1, $$, $1);}
    | VarDec COMMA ExtDecList {$$ = create_syntax(0, "ExtDecList", @$.first_line); add_node(3, $$, $1, $2, $3);}
    | VarDec error ExtDecList {yyerror("Missing ,", @1.first_line); yyerrok; yyclearin; }
    | VarDec error {yyerror("Missing ;", @1.first_line); yyerrok; yyclearin; }
    ;

// Specifiers
Specifier : TYPE {$$ = create_syntax(0, "Specifier", @$.first_line); add_node(1, $$, $1);}
    | StructSpecifier {$$ = create_syntax(0, "Specifier", @$.first_line); add_node(1, $$, $1);}
    ;

StructSpecifier : STRUCT OptTag LC DefList RC {$$ = create_syntax(0, "StructSpecifier", @$.first_line); add_node(5, $$, $1, $2, $3, $4, $5);}
    | STRUCT Tag {$$ = create_syntax(0, "StructSpecifier", @$.first_line); add_node(2, $$, $1, $2);}
    | STRUCT OptTag LC error RC {yyerror("Wrong StructSpecifier", @2.first_line); yyerrok; yyclearin; }
    ;

OptTag : ID {$$ = create_syntax(0, "OptTag", @$.first_line); add_node(1, $$, $1);}
    | %empty {$$ = NULL;}
    ;

Tag : ID {$$ = create_syntax(0, "Tag", @$.first_line); add_node(1, $$, $1);}
    ;


// Declarators
VarDec : ID {$$ = create_syntax(0, "VarDec", @$.first_line); add_node(1, $$, $1);}
    | VarDec LB INT RB {$$ = create_syntax(0, "VarDec", @$.first_line); add_node(4, $$, $1, $2, $3, $4);}
    | VarDec LB error RB {yyerror("Wrong VarDec", @3.first_line); yyerrok; yyclearin; }
    | VarDec LB error {yyerror("Missing RB", @2.first_line); yyerrok; yyclearin; }
    ;

FunDec : ID LP VarList RP {$$ = create_syntax(0, "FunDec", @$.first_line); add_node(4, $$, $1, $2, $3, $4);}
    | ID LP RP {$$ = create_syntax(0, "FunDec", @$.first_line); add_node(3, $$, $1, $2, $3);}
    | ID LP error RP {yyerror("Wrong FunDec", @3.first_line); yyerrok; yyclearin; }
    | error LP VarList RP {yyerror("Wrong FunName", @1.first_line); yyerrok; yyclearin;}
    ;

VarList : ParamDec COMMA VarList {$$ = create_syntax(0, "VarList", @$.first_line); add_node(3, $$, $1, $2, $3);}
    | ParamDec {$$ = create_syntax(0, "VarList", @$.first_line); add_node(1, $$, $1);}
    ;

ParamDec : Specifier VarDec {$$ = create_syntax(0, "ParamDec", @$.first_line); add_node(2, $$, $1, $2);}
    ;


// Statements
CompSt : LC DefList StmtList RC {$$ = create_syntax(0, "CompSt", @$.first_line); add_node(4, $$, $1, $2, $3, $4);}
    | LC error RC {yyerror("Wrong Compst", @1.first_line); yyerrok; yyclearin; }
    ;

StmtList : Stmt StmtList {$$ = create_syntax(0, "StmtList", @$.first_line); add_node(2, $$, $1, $2);}
    | %empty {$$ = NULL;}
    ;

Stmt : Exp SEMI {$$ = create_syntax(0, "Stmt", @$.first_line); add_node(2, $$, $1, $2);}
    | CompSt {$$ = create_syntax(0, "Stmt", @$.first_line); add_node(1, $$, $1);}
    | RETURN Exp SEMI {$$ = create_syntax(0, "Stmt", @$.first_line); add_node(3, $$, $1, $2, $3);}
    | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE {$$ = create_syntax(0, "Stmt", @$.first_line); add_node(5, $$, $1, $2, $3, $4, $5);}
    | IF LP Exp RP Stmt ELSE Stmt {$$ = create_syntax(0, "Stmt", @$.first_line); add_node(7, $$, $1, $2, $3, $4, $5, $6, $7);}
    | WHILE LP Exp RP Stmt {$$ = create_syntax(0, "Stmt", @$.first_line); add_node(5, $$, $1, $2, $3, $4, $5);}   
    | error SEMI {yyerror("Wrong Stmt", @1.first_line); yyerrok; yyclearin; }
    | Exp error {yyerror("Missing SEMI", @1.first_line); yyerrok; yyclearin; }
    | RETURN Exp error {yyerror("Missing SEMI", @3.first_line); yyerrok; yyclearin; }
    | RETURN error SEMI {yyerror("Wrong return", @2.first_line);}
    ;


// Local
DefList : Def DefList {$$ = create_syntax(0, "DefList", @$.first_line); add_node(2, $$, $1, $2);}
    | %empty {$$ = NULL;}
    ;

Def : Specifier DecList SEMI {$$ = create_syntax(0, "Def", @$.first_line); add_node(3, $$, $1, $2, $3);}
    | Specifier error SEMI {yyerror("Wrong Def", @2.first_line);}
    ;

DecList : Dec {$$ = create_syntax(0, "DecList", @$.first_line); add_node(1, $$, $1);}
    | Dec COMMA DecList {$$ = create_syntax(0, "DecList", @$.first_line); add_node(3, $$, $1, $2, $3);}
    | Dec error DecList {yyerror("Missing ','", @1.first_line);}
    | Dec error {yyerror("Wrong DecList", @2.first_line); yyerrok;}
    ;

Dec : VarDec {$$ = create_syntax(0, "Dec", @$.first_line); add_node(1, $$, $1);}
    | VarDec ASSIGNOP Exp {$$ = create_syntax(0, "Dec", @$.first_line); add_node(3, $$, $1, $2, $3);}
    | VarDec ASSIGNOP error {yyerror("Wrong Dec", @3.first_line); yyerrok; yyclearin; }
    ;

Exp : Exp ASSIGNOP Exp {$$ = create_syntax(0, "Exp", @$.first_line); add_node(3, $$, $1, $2, $3);}
    | Exp AND Exp {$$ = create_syntax(0, "Exp", @$.first_line); add_node(3, $$, $1, $2, $3);}
    | Exp OR Exp {$$ = create_syntax(0, "Exp", @$.first_line); add_node(3, $$, $1, $2, $3);}
    | Exp RELOP Exp {$$ = create_syntax(0, "Exp", @$.first_line); add_node(3, $$, $1, $2, $3);}
    | Exp PLUS Exp {$$ = create_syntax(0, "Exp", @$.first_line); add_node(3, $$, $1, $2, $3);}
    | Exp MINUS Exp {$$ = create_syntax(0, "Exp", @$.first_line); add_node(3, $$, $1, $2, $3);}
    | Exp STAR Exp {$$ = create_syntax(0, "Exp", @$.first_line); add_node(3, $$, $1, $2, $3);}
    | Exp DIV Exp {$$ = create_syntax(0, "Exp", @$.first_line); add_node(3, $$, $1, $2, $3);}
    | LP Exp RP {$$ = create_syntax(0, "Exp", @$.first_line); add_node(3, $$, $1, $2, $3);}
    | MINUS Exp {$$ = create_syntax(0, "Exp", @$.first_line); add_node(2, $$, $1, $2);}
    | NOT Exp {$$ = create_syntax(0, "Exp", @$.first_line); add_node(2, $$, $1, $2);}
    | ID LP Args RP {$$ = create_syntax(0, "Exp", @$.first_line); add_node(4, $$, $1, $2, $3, $4);}
    | ID LP RP {$$ = create_syntax(0, "Exp", @$.first_line); add_node(3, $$, $1, $2, $3);}
    | Exp LB Exp RB {$$ = create_syntax(0, "Exp", @$.first_line); add_node(4, $$, $1, $2, $3, $4);}
    | Exp DOT ID {$$ = create_syntax(0, "Exp", @$.first_line); add_node(3, $$, $1, $2, $3);}
    | ID {$$ = create_syntax(0, "Exp", @$.first_line); add_node(1, $$, $1);}
    | INT {$$ = create_syntax(0, "Exp", @$.first_line); add_node(1, $$, $1);}
    | FLOAT {$$ = create_syntax(0, "Exp", @$.first_line); add_node(1, $$, $1);}
    | Exp ASSIGNOP error    {yyerror("Wrong expression", @3.first_line); yyerrok;}
    | Exp AND error         {yyerror("Wrong expression", @3.first_line); yyerrok;}
    | Exp OR error          {yyerror("Wrong expression", @3.first_line); yyerrok;}
    | Exp RELOP error       {yyerror("Wrong expression", @3.first_line); yyerrok;}
    | Exp PLUS error        {yyerror("Wrong expression", @3.first_line); yyerrok;}
    | Exp MINUS error       {yyerror("Wrong expression", @3.first_line); yyerrok;}
    | Exp STAR error        {yyerror("Wrong expression", @3.first_line); yyerrok;}
    | Exp DIV error         {yyerror("Wrong expression", @3.first_line); yyerrok;}
    | LP error RP           {yyerror("Wrong expression", @2.first_line);}
    | MINUS error           {yyerror("Wrong expression", @2.first_line); yyerrok;}
    | NOT error             {yyerror("Wrong expression", @2.first_line); yyerrok;}
    | ID LP error RP        {yyerror("Wrong argument(s)", @3.first_line); yyerrok;}
    | ID LP error SEMI      {yyerror("Missing \")\"", @3.first_line);}
    | Exp LB error RB       {yyerror("Missing \"]\"", @3.first_line);}
    | Exp LB error SEMI     {yyerror("Missing \"]\"", @3.first_line);}
    ;
Args : Exp COMMA Args {$$ = create_syntax(0, "Args", @$.first_line); add_node(3, $$, $1, $2, $3);}
    | Exp {$$ = create_syntax(0, "Args", @$.first_line); add_node(1, $$, $1);}
    ;

%%

void yyerror(char* msg, ...)
{
    has_error = 1;
    if (msg[0] == 's' && msg[1] == 'y')
    {
//        printf("Error type B at Line %d: %s.", yylineno, msg);
        return;
    }
    else
    {
        va_list list;
        va_start(list, msg);
        printf("Error type B at Line %d: %s.\n", va_arg(list, int), msg);
        va_end(list);
    }
}