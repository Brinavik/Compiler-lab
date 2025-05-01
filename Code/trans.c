#include "lib.h"
#include "symbol.h"
#include "trans.h"


Codelist codelist;
void codelist_append(Code* code) {
    codelist.tail->next = code;
    codelist.tail = code;
}

void InitBasicComponents(){
    // init the symbol-table
    table = create_symbol_table();
    
    
    // insert the default function "read" and "write"
    char* ReadStr = (char*)malloc(5 * sizeof(char));
    char* WriteStr = (char*)malloc(6 * sizeof(char));
    strcpy(ReadStr, "read\0");
    strcpy(WriteStr, "write\0");
    Type IntType = create_basic(0);
    Type ReadType = create_func(IntType, ReadStr);
    Type WriteType = create_func(IntType, WriteStr);
    // Write para : INT , not insert yet. Maybe does not affect lab-3.
    insert_function_symbol(ReadStr, 0, ReadType);
    insert_function_symbol(WriteStr, 0, WriteType);


    // init codelist for "trans.h"
    codelist.head = (Code*)malloc(sizeof(Code)); 
    codelist.head->next = NULL;
    codelist.tail = codelist.head;                
}

char* new_temp() {
    static unsigned int temp_counter = 1;       
    char* name = (char*)malloc(sizeof(char) * MAX_CODE_LENGTH);
    snprintf(name, 16, "t%d", temp_counter++);
    return name;
}

char* new_label() {
    static unsigned int label_counter = 1;       
    char* name = (char*)malloc(sizeof(char) * MAX_CODE_LENGTH);
    snprintf(name, 16, "L%d", label_counter++);
    return name;
}

char* new_alias(){
    static unsigned int alias_counter = 1;       
    char* name = (char*)malloc(sizeof(char) * MAX_CODE_LENGTH);
    snprintf(name, 16, "v%d", alias_counter++);
    return name;
}

void print_intermediate_code() {
    Code* code = codelist.head->next;
    while(code) {
        printf("%s", code->str);
        code = code->next;
    }
}

void Trans_Program() {
#if TRANS_PRINT_DEBUG
    printf("Enter Trans_Program\n");
#endif
    Trans_ExtDefList(root->child[0]);
    print_intermediate_code();
}

void Trans_ExtDefList(Node* node) {
#if TRANS_PRINT_DEBUG
    printf("Enter Trans_ExtDefList\n");
#endif
    if (node == NULL) return;
    Trans_ExtDef(node->child[0]);
    Trans_ExtDefList(node->child[1]);
}

void Trans_ExtDef(Node* node) {
#if TRANS_PRINT_DEBUG
    printf("Enter Trans_ExtDef\n");
#endif
    if (strcmp(node->child[1]->name, "ExtDecList") == 0) {
        Trans_ExtDecList(node->child[1]);
    } else if (strcmp(node->child[1]->name, "FunDec") == 0) {
        Trans_FunDec(node->child[1]);
        Trans_CompSt(node->child[2]);
    }
}

void Trans_ExtDecList(Node* node) {
#if TRANS_PRINT_DEBUG
    printf("Enter Trans_ExtDecList\n");
#endif
    if(node->num == 1)
        Trans_VarDec(node->child[0]);
    else if(node->num == 3){
        Trans_VarDec(node->child[0]);
        Trans_ExtDecList(node->child[2]);
    }
}

void Trans_CompSt(Node* node) {
#if TRANS_PRINT_DEBUG
    printf("Enter Trans_CompSt\n");
#endif
    Trans_DefList(node->child[1]);
    Trans_StmtList(node->child[2]);
}

void Trans_StmtList(Node* node) {
#if TRANS_PRINT_DEBUG
    printf("Enter Trans_StmtList\n");
#endif
    if (node != NULL) {
        Trans_Stmt(node->child[0]);
        Trans_StmtList(node->child[1]);
    }
}

void Trans_Cond(Node* node, char* label_true, char* label_false) {
#if TRANS_PRINT_DEBUG
    printf("Enter Trans_Cond\n");
#endif
    if (strcmp(node->child[1]->name, "RELOP") == 0) {
        char* t1 = new_temp();
        char* t2 = new_temp();
        Trans_Exp(node->child[0], t1);
        Trans_Exp(node->child[2], t2);
        
#if TRANS_PRINT_DEBUG
        // 添加调试信息
        Code* debug_code = (Code*)malloc(sizeof(Code));
        sprintf(debug_code->str, "Print in func Trans_Cond case 1\n");
        codelist_append(debug_code);
#endif
        
        Code* code1 = (Code*)malloc(sizeof(Code));
        sprintf(code1->str, "IF %s %s %s GOTO %s\n", t1, node->child[1]->attr, t2, label_true);
        codelist_append(code1);

        Code* code2 = (Code*)malloc(sizeof(Code));
        sprintf(code2->str, "GOTO %s\n", label_false);
        codelist_append(code2);
    } else if (strcmp(node->child[0]->name, "NOT") == 0) {
        Trans_Cond(node->child[1], label_false, label_true);
    } else if (strcmp(node->child[1]->name, "AND") == 0) {
        char* label = new_label();
        Trans_Cond(node->child[0], label, label_false);
        
#if TRANS_PRINT_DEBUG
        // 添加调试信息
        Code* debug_code = (Code*)malloc(sizeof(Code));
        sprintf(debug_code->str, "Print in func Trans_Cond case 2\n");
        codelist_append(debug_code);
#endif
        
        Code* code1 = (Code*)malloc(sizeof(Code));
        sprintf(code1->str, "LABEL %s :\n", label);
        codelist_append(code1);
        Trans_Cond(node->child[2], label_true, label_false);
    } else if (strcmp(node->child[1]->name, "OR") == 0) {
        char* label = new_label();
        Trans_Cond(node->child[0], label_true, label);
        
#if TRANS_PRINT_DEBUG
        // 添加调试信息
        Code* debug_code = (Code*)malloc(sizeof(Code));
        sprintf(debug_code->str, "Print in func Trans_Cond case 3\n");
        codelist_append(debug_code);
#endif
        
        Code* code1 = (Code*)malloc(sizeof(Code));
        sprintf(code1->str, "LABEL %s :\n", label);
        codelist_append(code1);
        Trans_Cond(node->child[2], label_true, label_false);
    } else {
        char* t1 = new_temp();
        Trans_Exp(node, t1);
        
#if TRANS_PRINT_DEBUG
        // 添加调试信息
        Code* debug_code = (Code*)malloc(sizeof(Code));
        sprintf(debug_code->str, "Print in func Trans_Cond case 4\n");
        codelist_append(debug_code);
#endif
        
        Code* code1 = (Code*)malloc(sizeof(Code));
        sprintf(code1->str, "IF %s != #0 GOTO %s\n", t1, label_true);
        codelist_append(code1);
        Code* code2 = (Code*)malloc(sizeof(Code));
        sprintf(code2->str, "GOTO %s\n", label_false);
        codelist_append(code2);
    }
}

void Trans_Stmt(Node* node) {
#if TRANS_PRINT_DEBUG
    printf("Enter Trans_Stmt\n");
#endif
    if(!node)   return;
    if (node->num == 2) {
        Trans_Exp(node->child[0], NULL);
    } else if (node->num == 1) {
        Trans_CompSt(node->child[0]);
    } else if (node->num == 3) { 
        char* t1 = new_temp();
        Trans_Exp(node->child[1], t1);
        
#if TRANS_PRINT_DEBUG
        // 添加调试信息
        Code* debug_code = (Code*)malloc(sizeof(Code));
        sprintf(debug_code->str, "Print in func Trans_Stmt case 1\n");
        codelist_append(debug_code);
#endif
        
        Code* code2 = (Code*)malloc(sizeof(Code));
        sprintf(code2->str, "RETURN %s\n", t1);
        codelist_append(code2);
    } else if (node->num == 5 && strcmp(node->child[0]->name, "WHILE") == 0 ){ 
        char* label1 = new_label();
        char* label2 = new_label();
        char* label3 = new_label();

#if TRANS_PRINT_DEBUG
        // 添加调试信息
        Code* debug_code = (Code*)malloc(sizeof(Code));
        sprintf(debug_code->str, "Print in func Trans_Stmt case 2\n");
        codelist_append(debug_code);
#endif
        
        Code* code1 = (Code*)malloc(sizeof(Code));
        sprintf(code1->str, "LABEL %s :\n", label1);
        codelist_append(code1);
        Trans_Cond(node->child[2], label2, label3);

        Code* code2 = (Code*)malloc(sizeof(Code));
        sprintf(code2->str, "LABEL %s :\n", label2);
        codelist_append(code2);
        Trans_Stmt(node->child[4]);

        Code* code3 = (Code*)malloc(sizeof(Code));
        sprintf(code3->str, "GOTO %s\n", label1);
        codelist_append(code3);

        Code* code4 = (Code*)malloc(sizeof(Code));
        sprintf(code4->str, "LABEL %s :\n", label3);
        codelist_append(code4);
    } else if (node->num == 5 && strcmp(node->child[0]->name, "IF") == 0 ){
        char* label1 = new_label();
        char* label2 = new_label();

        Trans_Cond(node->child[2], label1, label2);

#if TRANS_PRINT_DEBUG
        // 添加调试信息
        Code* debug_code = (Code*)malloc(sizeof(Code));
        sprintf(debug_code->str, "Print in func Trans_Stmt case 3\n");
        codelist_append(debug_code);
#endif
        
        Code* code_label1 = (Code*)malloc(sizeof(Code));
        sprintf(code_label1->str, "LABEL %s :\n", label1);
        codelist_append(code_label1);

        Trans_Stmt(node->child[4]); 

        Code* code_label2 = (Code*)malloc(sizeof(Code));
        sprintf(code_label2->str, "LABEL %s :\n", label2);
        codelist_append(code_label2);
    } else if (node->num == 7) { 
        char* label1 = new_label();
        char* label2 = new_label();
        char* label3 = new_label();

        Trans_Cond(node->child[2], label1, label2);

        Code* code_label1 = (Code*)malloc(sizeof(Code));
        sprintf(code_label1->str, "LABEL %s :\n", label1);
        codelist_append(code_label1);

        Trans_Stmt(node->child[4]);  

        Code* code_goto = (Code*)malloc(sizeof(Code));
        sprintf(code_goto->str, "GOTO %s\n", label3);
        codelist_append(code_goto);

        Code* code_label2 = (Code*)malloc(sizeof(Code));
        sprintf(code_label2->str, "LABEL %s :\n", label2);
        codelist_append(code_label2);

        Trans_Stmt(node->child[6]);  

        Code* code_label3 = (Code*)malloc(sizeof(Code));
        sprintf(code_label3->str, "LABEL %s :\n", label3);
        codelist_append(code_label3);
    } else  // Unknown Case
        assert(0);
}

void Trans_DefList(Node* node) {
#if TRANS_PRINT_DEBUG
    printf("Enter Trans_DefList\n");
#endif
    if (node != NULL) {
        Trans_Def(node->child[0]);
        Trans_DefList(node->child[1]);
    }
}

void Trans_Def(Node* node) {
#if TRANS_PRINT_DEBUG
    printf("Enter Trans_Def\n");
#endif
    Trans_DecList(node->child[1]);
}

void Trans_DecList(Node* node) {
#if TRANS_PRINT_DEBUG
    printf("Enter Trans_DecList\n");
#endif
    if (node->num == 1) {
        Trans_Dec(node->child[0]);
    } else if(node->num == 3){
        Trans_Dec(node->child[0]);
        Trans_DecList(node->child[2]);
    }
}

void Trans_Args(Node* node, Arg** arg_list_head, Arg** arg_list_tail) {
#if TRANS_PRINT_DEBUG
    printf("Enter Trans_Args\n");
#endif
    if (node->num == 1) {  // Case: Exp 
        char* t1 = new_temp();
        Trans_Exp(node->child[0], t1);  

        Arg* new_arg = (Arg*)malloc(sizeof(Arg));
        new_arg->name = t1;
        new_arg->next = NULL;

        if (*arg_list_head == NULL) {
            *arg_list_head = new_arg;
            *arg_list_tail = new_arg;
        } else {
            (*arg_list_tail)->next = new_arg;
            *arg_list_tail = new_arg;
        }

    } else if (node->num == 3) {  // Case: Exp COMMA Args1 
        char* t1 = new_temp();
        Trans_Exp(node->child[0], t1);

        Arg* new_arg = (Arg*)malloc(sizeof(Arg));
        new_arg->name = t1;
        new_arg->next = NULL;

        if (*arg_list_head == NULL) {
            *arg_list_head = new_arg;
            *arg_list_tail = new_arg;
        } else {
            (*arg_list_tail)->next = new_arg;
            *arg_list_tail = new_arg;
        }

        Trans_Args(node->child[2], arg_list_head, arg_list_tail);
    }
}

void Trans_Exp(Node* node, char* place) {
#if TRANS_PRINT_DEBUG
    printf("Enter Trans_Exp\n");
#endif
    if(node->num == 1){
        if (strcmp(node->child[0]->name, "INT") == 0) {
#if TRANS_PRINT_DEBUG
            // 添加调试信息
            Code* debug_code = (Code*)malloc(sizeof(Code));
            sprintf(debug_code->str, "Print in func Trans_Exp case 1\n");
            codelist_append(debug_code);
#endif
            
            Code* code1 = (Code*)malloc(sizeof(Code));
            sprintf(code1->str, "%s := #%s\n", place, node->child[0]->attr);
            codelist_append(code1);
        } else if (strcmp(node->child[0]->name, "ID") == 0) {
#if TRANS_PRINT_DEBUG
            // 添加调试信息
            Code* debug_code = (Code*)malloc(sizeof(Code));
            sprintf(debug_code->str, "Print in func Trans_Exp case 2\n");
            codelist_append(debug_code);
#endif
            SymbolEntry* entry = lookup_symbol(node->child[0]->attr);
            assert(entry != NULL);
            if (entry->type->kind == ARRAY) {  // 判断是否为数组类型
            // 生成数组地址
                Code* code1 = (Code*)malloc(sizeof(Code));
                if (entry->is_param) { // 参数数组直接使用地址
                    sprintf(code1->str, "%s := %s\n", place, entry->name); 
                } else { // 局部数组需要取地址
                    sprintf(code1->str, "%s := &%s\n", place, entry->name);
                }
                codelist_append(code1);
            } else{
            Code* code1 = (Code*)malloc(sizeof(Code));
            sprintf(code1->str, "%s := %s\n", place, node->child[0]->attr);
            codelist_append(code1);
            }
        } else if (strcmp(node->child[0]->name, "FLOAT") == 0){
            assert(0);
        }
    } 
    
#if TRANS_ARRAY_SUPPORT
    else if (node->num == 4 && strcmp(node->child[1]->name, "LB") == 0) {
        // 数组访问处理（仅当启用数组支持时）
        char* t_addr = new_temp();
        Trans_Exp_Addr(node, t_addr); 

        #if TRANS_PRINT_DEBUG
        // 添加调试信息
        Code* debug_code = (Code*)malloc(sizeof(Code));
        sprintf(debug_code->str, "Print in func Trans_Exp case 11\n");
        codelist_append(debug_code);
#endif
        char* result = new_temp();
        Code* code_load = (Code*)malloc(sizeof(Code));
        sprintf(code_load->str, "%s := *%s\n", result, t_addr);
        codelist_append(code_load);

        if (place != NULL) {
            Code* code_assign = (Code*)malloc(sizeof(Code));
            sprintf(code_assign->str, "%s := %s\n", place, result);
            codelist_append(code_assign);
        }
    } 
#endif

    
    else if (strcmp(node->child[0]->name, "MINUS") == 0) {
        char* t1 = new_temp();
        Trans_Exp(node->child[1], t1);
        
#if TRANS_PRINT_DEBUG
        // 添加调试信息
        Code* debug_code = (Code*)malloc(sizeof(Code));
        sprintf(debug_code->str, "Print in func Trans_Exp case 3\n");
        codelist_append(debug_code);
#endif
        
        Code* code2 = (Code*)malloc(sizeof(Code));
        sprintf(code2->str, "%s := #0 - %s\n", place, t1);
        codelist_append(code2);
    } else if(node->num == 3 || (node->num == 2 && strcmp(node->child[0]->name, "NOT") == 0)){
        if(strcmp(node->child[0]->name, "LP") == 0) {
            Trans_Exp(node->child[1], place);
        } else if (strcmp(node->child[1]->name, "ASSIGNOP") == 0) {

#if TRANS_ARRAY_SUPPORT
        Node* left_exp = node->child[0];
#if TRANS_PRINT_DEBUG
                // 添加调试信息
                Code* debug_code = (Code*)malloc(sizeof(Code));
                sprintf(debug_code->str, "Print in func Trans_Exp case 4\n");
                codelist_append(debug_code);
#endif
        // 情况1：左值是普通变量（ID节点且不是数组访问）
        if (left_exp->num == 1 && strcmp(left_exp->child[0]->name, "ID") == 0) {
            // 直接赋值给普通变量
            char* right_val = new_temp();
            Trans_Exp(node->child[2], right_val);
            Code* code_assign = (Code*)malloc(sizeof(Code));
            sprintf(code_assign->str, "%s := %s\n", left_exp->child[0]->attr, right_val);
            codelist_append(code_assign);
            if (place != NULL) {
                Code* code_place = (Code*)malloc(sizeof(Code));
                sprintf(code_place->str, "%s := %s\n", place, right_val);
                codelist_append(code_place);
            }
        }
        // 情况2：左值是数组元素或其他需要地址的情况
        else {
            char* left_addr = new_temp();
            Trans_Exp_Addr(left_exp, left_addr);
            char* right_val = new_temp();
            Trans_Exp(node->child[2], right_val);
            Code* code_store = (Code*)malloc(sizeof(Code));
            sprintf(code_store->str, "*%s := %s\n", left_addr, right_val);
            codelist_append(code_store);
            if (place != NULL) {
                Code* code_assign = (Code*)malloc(sizeof(Code));
                sprintf(code_assign->str, "%s := %s\n", place, right_val);
                codelist_append(code_assign);
            }
        }
#else
            if (node->child[0]->num == 1 && strcmp(node->child[0]->child[0]->name, "ID") == 0) {
                char* t1 = new_temp();
                Trans_Exp(node->child[2], t1);
                
#if TRANS_PRINT_DEBUG
                // 添加调试信息
                Code* debug_code = (Code*)malloc(sizeof(Code));
                sprintf(debug_code->str, "Print in func Trans_Exp case 4\n");
                codelist_append(debug_code);
#endif
                Code* code2_1 = (Code*)malloc(sizeof(Code));
            /*
                SymbolEntry* entry = lookup_symbol(node->child[0]->child[0]->attr);
                assert(entry != NULL);
                sprintf(code2_1->str, "%s := %s\n", entry->alias, t1);
            */    
                sprintf(code2_1->str, "%s := %s\n", node->child[0]->child[0]->attr, t1);
                codelist_append(code2_1);
                if(place != NULL) {
                Code* code2_2 = (Code*)malloc(sizeof(Code));
                sprintf(code2_2->str, "%s := %s\n", place, node->child[0]->child[0]->attr);
                codelist_append(code2_2);
                }
            } else {
                assert(0);
        } 
#endif
        }
        
        else if (strcmp(node->child[1]->name, "PLUS") == 0 || strcmp(node->child[1]->name, "MINUS") == 0 || 
                   strcmp(node->child[1]->name, "STAR") == 0 || strcmp(node->child[1]->name, "DIV") == 0 ) {
            char* t1 = new_temp();
            char* t2 = new_temp();
            Trans_Exp(node->child[0], t1);
            Trans_Exp(node->child[2], t2);
            
#if TRANS_PRINT_DEBUG
            // 添加调试信息
            Code* debug_code = (Code*)malloc(sizeof(Code));
            sprintf(debug_code->str, "Print in func Trans_Exp case 5\n");
            codelist_append(debug_code);
#endif
            
            Code* code3 = (Code*)malloc(sizeof(Code));

            if(strcmp(node->child[1]->name, "PLUS") == 0)
                sprintf(code3->str, "%s := %s + %s\n", place, t1, t2);
            else if(strcmp(node->child[1]->name, "MINUS") == 0)
                sprintf(code3->str, "%s := %s - %s\n", place, t1, t2);
            else if(strcmp(node->child[1]->name, "STAR") == 0)
                sprintf(code3->str, "%s := %s * %s\n", place, t1, t2);
            else if(strcmp(node->child[1]->name, "DIV") == 0)
                sprintf(code3->str, "%s := %s / %s\n", place, t1, t2);

            codelist_append(code3);
        } else if (strcmp(node->child[1]->name, "RELOP") == 0 || strcmp(node->child[1]->name, "AND") == 0 || 
                   strcmp(node->child[1]->name, "OR") == 0 || strcmp(node->child[0]->name, "NOT") == 0) {
            char* label1 = new_label();
            char* label2 = new_label();

            Code* code0 = (Code*)malloc(sizeof(Code));
            sprintf(code0->str, "%s := #0\n", place);
            codelist_append(code0);

            Trans_Cond(node, label1, label2);

            Code* code1 = (Code*)malloc(sizeof(Code));
            sprintf(code1->str, "LABEL %s :\n", label1);
            codelist_append(code1);

            Code* code2 = (Code*)malloc(sizeof(Code));
            sprintf(code2->str, "%s := #1\n", place);
            codelist_append(code2);

            Code* code3 = (Code*)malloc(sizeof(Code));
            sprintf(code3->str, "LABEL %s :\n", label2);
            codelist_append(code3);
        } else if (strcmp(node->child[0]->name, "ID") == 0) {
            if (strcmp(node->child[0]->attr, "read") == 0) {
    #if TRANS_PRINT_DEBUG
                // 添加调试信息
                Code* debug_code = (Code*)malloc(sizeof(Code));
                sprintf(debug_code->str, "Print in func Trans_Exp case 6\n");
                codelist_append(debug_code);
    #endif
                
                Code* code1 = (Code*)malloc(sizeof(Code));
                sprintf(code1->str, "READ %s\n", place);
                codelist_append(code1);
            } else {
    #if TRANS_PRINT_DEBUG
                // 添加调试信息
                Code* debug_code = (Code*)malloc(sizeof(Code));
                sprintf(debug_code->str, "Print in func Trans_Exp case 7\n");
                codelist_append(debug_code);
    #endif
                
                Code* code2 = (Code*)malloc(sizeof(Code));
                sprintf(code2->str, "%s := CALL %s\n", place, node->child[0]->attr);
                codelist_append(code2);
            } 
        }
    } 
    
#if TRANS_ARRAY_SUPPORT   
else if (node->num == 4 && strcmp(node->child[0]->name, "ID") == 0) { // ID LP Args RP
        Arg* arg_list_head = NULL;
        Arg* arg_list_tail = NULL;
        Trans_Args(node->child[2], &arg_list_head, &arg_list_tail);

        if (strcmp(node->child[0]->attr, "write") == 0) {
    #if TRANS_PRINT_DEBUG
            // 添加调试信息
            Code* debug_code = (Code*)malloc(sizeof(Code));
            sprintf(debug_code->str, "Print in func Trans_Exp case 8\n");
            codelist_append(debug_code);
    #endif

            Code* code1 = (Code*)malloc(sizeof(Code));
            sprintf(code1->str, "WRITE %s\n", arg_list_head->name);
            codelist_append(code1);
            if(place != NULL) {
                Code* code2 = (Code*)malloc(sizeof(Code));
                sprintf(code2->str, "%s := #0\n", place);
                codelist_append(code2);
            }
        } else {
            Arg* prev = NULL;
            Arg* curr = arg_list_head;
            while (curr) {
                Arg* next = curr->next;
                curr->next = prev;
                prev = curr;
                curr = next;
            }
            arg_list_head = prev;

            Arg* cur = arg_list_head;
            while (cur) {
    #if TRANS_PRINT_DEBUG
                // 添加调试信息
                Code* debug_code = (Code*)malloc(sizeof(Code));
                sprintf(debug_code->str, "Print in func Trans_Exp case 9\n");
                codelist_append(debug_code);
    #endif
        /* 关键修改开始：参数类型检查 */
        SymbolEntry* entry = lookup_symbol(cur->name);
        Code* code_arg = (Code*)malloc(sizeof(Code));
        if (entry && entry->type->kind == ARRAY) {
            if (entry->is_param) {
                // 参数数组直接传递名称（已在函数参数中处理地址）
                sprintf(code_arg->str, "ARG %s\n", cur->name);
            } else {
                // 局部数组需要传递地址
                char* addr = new_temp();
                Trans_Exp_Addr(node, addr);
                sprintf(code_arg->str, "ARG %s\n", addr);
            }
        } else {
            // 普通参数传值
            sprintf(code_arg->str, "ARG %s\n", cur->name);
        }
        /* 关键修改结束 */

        cur = cur->next;
        }

            Code* code_call = (Code*)malloc(sizeof(Code));
            sprintf(code_call->str, "%s := CALL %s\n", place, node->child[0]->attr);
            codelist_append(code_call);
        }
    }
#else 


#endif
    else // Known case
        assert(0);
}


void Trans_FunDec(Node* node) {
#if TRANS_PRINT_DEBUG
    printf("Enter Trans_FunDec\n");
#endif

#if TRANS_PRINT_DEBUG    
    Code* debug_code = (Code*)malloc(sizeof(Code));
    sprintf(debug_code->str, "Print in func Trans_FunDec case 1\n");
    codelist_append(debug_code);
#endif     

    char* func_name = node->child[0]->attr;
    Code* func_code = (Code*)malloc(sizeof(Code));
    sprintf(func_code->str, "FUNCTION %s :\n", func_name);
    codelist_append(func_code);

    if(node->num == 3) {    
        return;
    } else if(node->num == 4){    
        Trans_VarList(node->child[2]);
    }
    else // Unknown case
        assert(0);
}

void Trans_VarList(Node* node) {
#if TRANS_PRINT_DEBUG
    printf("Enter Trans_VarList\n");
#endif
    if(node->num == 1) {    
        Trans_ParamDec(node->child[0]);
    } else if(node->num == 3) {    
        Trans_ParamDec(node->child[0]);
        Trans_VarList(node->child[2]);
    }
    else 
        // Unknown case
        assert(0);
}


void Trans_Specifier(Node* node) {
#if TRANS_PRINT_DEBUG
    printf("Enter Trans_Specifier\n");
#endif
    // Specifier return ; And Specifiers below , also just return.
}

void Trans_StructSpecifier(Node* node) {
#if TRANS_PRINT_DEBUG
    printf("Enter Trans_StructSpecifier\n");
#endif
}

void Trans_OptTag(Node* node) {
#if TRANS_PRINT_DEBUG
    printf("Enter Trans_OptTag\n");
#endif
}

void Trans_Tag(Node* node) {
#if TRANS_PRINT_DEBUG
    printf("Enter Trans_Tag\n");
#endif
}

void Trans_Dec(Node* node) {
#if TRANS_PRINT_DEBUG
    printf("Enter Trans_Dec\n");
#endif
    if(node->num == 1)
        Trans_VarDec(node->child[0]);
    else if(node->num == 3) {
        Trans_VarDec(node->child[0]);
        char* t1 = new_temp();
        Trans_Exp(node->child[2], t1);
    }
}




void Trans_Exp_Addr(Node* node, char* place) {
    // Case 1：简单变量（ID）
    if (node->num == 1 && strcmp(node->child[0]->name, "ID") == 0) {
        SymbolEntry* entry = lookup_symbol(node->child[0]->attr);
        assert(entry != NULL);

        Code* code = (Code*)malloc(sizeof(Code));
        
        /* 改进点：如果是数组参数，直接使用参数地址 */
        if (entry->type->kind == ARRAY && entry->is_param == 1) {
            // 数组参数直接使用传入的地址
            sprintf(code->str, "%s := %s\n", place, node->child[0]->attr);
        } else {
            // 普通变量或局部数组显式取地址
            sprintf(code->str, "%s := &%s\n", place, node->child[0]->attr);
        }
        
        codelist_append(code);
    }
    
    // Case 2：数组访问（Exp LB Exp RB）
    else if (node->num == 4 && strcmp(node->child[1]->name, "LB") == 0) {
        // 步骤1：递归计算基地址（处理多维数组情况）
        char* t_base = new_temp();
        Trans_Exp_Addr(node->child[0], t_base); // 此时 t_base 已经是数组基地址
        
        // 步骤2：计算索引值
        char* t_index = new_temp();
        Trans_Exp(node->child[2], t_index);
        
        // 步骤3：计算偏移量（元素大小固定为4字节）
        char* t_offset = new_temp();
        Code* code_offset = (Code*)malloc(sizeof(Code));
        sprintf(code_offset->str, "%s := %s * #4\n", t_offset, t_index);
        codelist_append(code_offset);
        
        // 步骤4：计算最终地址
        Code* code_addr = (Code*)malloc(sizeof(Code));
        sprintf(code_addr->str, "%s := %s + %s\n", place, t_base, t_offset);
        codelist_append(code_addr);
    }
    
    // Case 3：结构体成员访问（可选，根据需求保留）
    // else if (...) { ... }
    
    else {
        assert(0); // 不支持的左值形式
    }
}

#if TRANS_ARRAY_SUPPORT
// 数组支持专用代码块
void Trans_VarDec(Node* node) {
#if TRANS_PRINT_DEBUG
    printf("Enter Trans_VarDec\n");
#endif
    if (node->num == 4 && strcmp(node->child[1]->name, "LB") == 0) {
        // 数组声明处理（类型检查假设已在语义分析完成）
        return;
    }
    if (node->num == 1 && strcmp(node->child[0]->name, "ID") == 0) {
        return;
    }
    assert(0);
}

void Trans_ParamDec(Node* node) {
    #if TRANS_PRINT_DEBUG
        printf("Enter Trans_ParamDec\n");
    #endif
    if (node->num == 2) {
        Trans_Specifier(node->child[0]);
        Node* var_dec = node->child[1];
        if (var_dec) {
            // 处理数组参数（如 int a[]）
            if (var_dec->num == 4 && strcmp(var_dec->child[1]->name, "LB") == 0) {
                char* param_name = var_dec->child[0]->attr;
                
                // 创建一个数组参数的变量，放入符号表
                Type elemType = create_basic(0);
                Type arrayType = create_array(elemType, atoi(var_dec->child[2]->attr));
                printf("param name : %s\n", param_name);
            //    assert(0);
                if(!insert_symbol(param_name, 0, arrayType))
                    assert(0);
                SymbolEntry* entry = lookup_symbol(param_name);
                entry->is_param = 1;

                // 传递地址
                Code* param_code = (Code*)malloc(sizeof(Code));
                sprintf(param_code->str, "PARAM %s\n", param_name);
                codelist_append(param_code);
            }
            // 处理普通变量
            else if (var_dec->num == 1 && strcmp(var_dec->child[0]->name, "ID") == 0) {
                char* param_name = var_dec->child[0]->attr;
                Code* param_code = (Code*)malloc(sizeof(Code));
                sprintf(param_code->str, "PARAM %s\n", param_name);
                codelist_append(param_code);

                // 插入符号表为基本类型
                insert_symbol(param_name, 0, create_basic(0));
                SymbolEntry* entry = lookup_symbol(param_name);
                entry->is_param = 1;
            }
        }
        Trans_VarDec(var_dec);
        }
    }
#else
// 原始代码保持不变
void Trans_ParamDec(Node* node) {
    #if TRANS_PRINT_DEBUG
        printf("Enter Trans_ParamDec\n");
    #endif
        if (node->num == 2) {
            Trans_Specifier(node->child[0]);
            Node* var_dec = node->child[1];
            if (var_dec->num == 1 && strcmp(var_dec->child[0]->name, "ID") == 0) {
                char* param_name = var_dec->child[0]->attr;
                
                Code* param_code = (Code*)malloc(sizeof(Code));
            /*
                SymbolEntry* entry = lookup_symbol(param_name);
                assert(entry != NULL);
                sprintf(param_code->str, "PARAM %s\n", entry->alias);
            */
                sprintf(param_code->str, "PARAM %s\n", param_name);
                codelist_append(param_code);
            }
            Trans_VarDec(var_dec);
        }
    }
    
void Trans_VarDec(Node* node) {
    #if TRANS_PRINT_DEBUG
        printf("Enter Trans_VarDec\n");
    #endif
        // ID	return
        // ID LB INT RB
    }
#endif