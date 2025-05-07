#include "lib.h"
#include "symbol.h"
#include "trans.h"

Codelist codelist;

static void insert_op(Code* code, char* opname, unsigned int opnum) {
    if (opname == NULL || strlen(opname) == 0) {
        code->ops[opnum].optype = OP_EMPTY;
        return;
    }
    switch (opname[0]) {
        case '0': case '1': case '2': case '3': case '4': 
        case '5': case '6': case '7': case '8': case '9':  
            code->ops[opnum].optype = OP_CONST;
            break;
        case '&':
            code->ops[opnum].optype = OP_ADDR;
            break;
        case '*':
            code->ops[opnum].optype = OP_DEREF;
            break;
        case 'L':
            code->ops[opnum].optype = OP_LABEL;
            break;
        case 't':
            code->ops[opnum].optype = OP_TEMP;
            break;
        default:
        // Variable.
            code->ops[opnum].optype = OP_TEMP;
            break;
    }    
    char* name = my_strdup(opname);
    code->ops[opnum].name = name;
}

static void insert_ops(Code* code, char* name0, char* name1, char* name2) {
    assert(code);
    /*
        Define 4 Optypes in trans.h.
        OP_CONST: like #3;
        OP_ADDR: like &x;
        OP_DEREF: like *y;
        OP_TEMP: like t1;
        I identify them in insert_op()'s definition. Make sure the input parameter formats are correct!  
    */
    insert_op(code, name0, 0);
    insert_op(code, name1, 1);
    insert_op(code, name2, 2);
}

void codelist_append(Code* code) {
    codelist.tail->next = code;
    codelist.tail = code;
    codelist.tail->next = NULL;
    
    /*
        Here I bind a special attribute (code->attr) to intermediate code of 
        different formats, which will be used for the MIPS-code generation in lab4.
    */
    
    char buffer[MAX_CODE_LENGTH];
    const char *str = code->str;
    strncpy(buffer, str, sizeof(buffer)-1);
    buffer[sizeof(buffer)-1] = '\0';

    // 预处理：去除末尾可能的冒号和空格
    char *end = buffer + strlen(buffer) - 1;
    while (end >= buffer && (*end == ' ' || *end == ':')) end--;
    *(end+1) = '\0';

    // 提取第一个单词
    char first_word[MAX_CODE_LENGTH] = {0};
    sscanf(buffer, "%s", first_word);

    // 优先匹配独立关键字
    if (strcmp(first_word, "LABEL") == 0) {
        code->codetype = CODE_LABEL;
        return;
    }
    if (strcmp(first_word, "FUNCTION") == 0) {
        code->codetype = CODE_FUNCTION;
        return;
    }
    if (strcmp(first_word, "GOTO") == 0) {  
        code->codetype = CODE_GOTO;
        return;
    }
    if (strcmp(first_word, "RETURN") == 0) {
        code->codetype = CODE_RETURN;
        return;
    }
    if (strcmp(first_word, "DEC") == 0) {
        code->codetype = CODE_DEC;
        return;
    }
    if (strcmp(first_word, "ARG") == 0) {
        code->codetype = CODE_ARG;
        return;
    }
    if (strcmp(first_word, "PARAM") == 0) {
        code->codetype = CODE_PARAM;
        return;
    }
    if (strcmp(first_word, "READ") == 0) {
        code->codetype = CODE_READ;
        return;
    }
    if (strcmp(first_word, "WRITE") == 0) {
        code->codetype = CODE_WRITE;
        return;
    }
    if (strcmp(first_word, "IF") == 0) {
        code->codetype = CODE_IFGOTO;
        return;
    }

    // 处理赋值语句
    char *assign_pos = strstr(buffer, ":=");
    if (assign_pos) {
        // 判断CALL语句
        if (strstr(assign_pos, "CALL") != NULL) {
            code->codetype = CODE_CALL;
            return;
        }

        // 解引用赋值 (*x := y)
        if (buffer[0] == '*' && is_valid_identifier_char(buffer[1])) {
            code->codetype = CODE_ASSIGN_DEREF;
            return;
        }

       // 检查右侧操作符（不再包含*）
        char *op_pos = strpbrk(assign_pos, "+-*/&");
        if (op_pos) {
            // 1. 先处理取地址操作
            if (strchr(assign_pos, '&')) {
                code->codetype = CODE_ASSIGN_ADDR;
                return;
            }

            // 2. 精确识别解引用操作
            char *star_pos = assign_pos;
            while ((star_pos = strchr(star_pos, '*')) != NULL) {
                // 解引用特征：*前是空格或运算符，*后是标识符
                if ((star_pos == assign_pos || 
                    strchr(" +-*/(,[", star_pos[-1])) &&
                    is_valid_identifier_char(star_pos[1])) {
                    code->codetype = CODE_ASSIGN_ADDR;
                    return;
                }
                star_pos++; // 继续查找下一个*
            }
                        
            // 3. 识别具体的操作符类型
            if (strchr(op_pos, '+')) {
                code->codetype = CODE_ADD;
                return;
            } else if (strchr(op_pos, '-')) {
                code->codetype = CODE_SUB;
                return;
            } else if (strchr(op_pos, '*')) {
                code->codetype = CODE_MUL;
                return;
            } else if (strchr(op_pos, '/')) {
                code->codetype = CODE_DIV;
                return;
            } else {
                // Unknown op
                assert(0);
            }
        }

        // 简单赋值
        code->codetype = CODE_SIMPLE_ASSIGNOP;
        return;
    }

    // 未匹配任何规则
    code->codetype = CODE_UNKNOWN;
}

// 辅助函数实现
static int is_valid_identifier_char(char c) {
    return isalnum(c) || c == '_';
}

static int is_relop_char(char c) {
    return strchr("=<>!", c) != NULL;
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
    append_fieldlist(WriteType, "", IntType);
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

void print_intermediate_code(char* output) {
    Code* code = codelist.head->next;
    FILE *file = fopen(output, "w");
    while(code) {
    #if !TRANS_MIPS_OUTPUT
        fprintf(file, "%s", code->str);
    #endif
        //    print_code_codetype(code);
        code = code->next;
    }
    fclose(file);
}

void Trans_Program(char* output) {
#if TRANS_PRINT_DEBUG
    printf("Enter Trans_Program\n");
#endif
    Trans_ExtDefList(root->child[0]);
    print_intermediate_code(output);
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
        insert_ops(debug_code, "", "", "");
        codelist_append(debug_code);
#endif
        
        Code* code1 = (Code*)malloc(sizeof(Code));
        sprintf(code1->str, "IF %s %s %s GOTO %s\n", t1, node->child[1]->attr, t2, label_true);
        insert_ops(code1, t1, t2, label_true);
        codelist_append(code1);

        Code* code2 = (Code*)malloc(sizeof(Code));
        sprintf(code2->str, "GOTO %s\n", label_false);
        insert_ops(code2, label_false, "", "");
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
        insert_ops(debug_code, "", "", "");
        codelist_append(debug_code);
#endif
        
        Code* code1 = (Code*)malloc(sizeof(Code));
        sprintf(code1->str, "LABEL %s :\n", label);
        insert_ops(code1, label, "", "");
        codelist_append(code1);
        Trans_Cond(node->child[2], label_true, label_false);
    } else if (strcmp(node->child[1]->name, "OR") == 0) {
        char* label = new_label();
        Trans_Cond(node->child[0], label_true, label);
        
#if TRANS_PRINT_DEBUG
        // 添加调试信息
        Code* debug_code = (Code*)malloc(sizeof(Code));
        sprintf(debug_code->str, "Print in func Trans_Cond case 3\n");
        insert_ops(debug_code, "", "", "");
        codelist_append(debug_code);
#endif
        
        Code* code1 = (Code*)malloc(sizeof(Code));
        sprintf(code1->str, "LABEL %s :\n", label);
        insert_ops(code1, label, "", "");
        codelist_append(code1);
        Trans_Cond(node->child[2], label_true, label_false);
    } else {
        char* t1 = new_temp();
        Trans_Exp(node, t1);
        
#if TRANS_PRINT_DEBUG
        // 添加调试信息
        Code* debug_code = (Code*)malloc(sizeof(Code));
        sprintf(debug_code->str, "Print in func Trans_Cond case 4\n");
        insert_ops(debug_code, "", "", "");
        codelist_append(debug_code);
#endif
        
        Code* code1 = (Code*)malloc(sizeof(Code));
        sprintf(code1->str, "IF %s != #0 GOTO %s\n", t1, label_true);
        insert_ops(code1, t1, label_true, "");
        codelist_append(code1);
        Code* code2 = (Code*)malloc(sizeof(Code));
        sprintf(code2->str, "GOTO %s\n", label_false);
        insert_ops(code2, label_false, "", "");
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
        insert_ops(debug_code, "", "", "");
        codelist_append(debug_code);
#endif
        
        Code* code2 = (Code*)malloc(sizeof(Code));
        sprintf(code2->str, "RETURN %s\n", t1);
        insert_ops(code2, t1, "", "");
        codelist_append(code2);
    } else if (node->num == 5 && strcmp(node->child[0]->name, "WHILE") == 0 ){ 
        char* label1 = new_label();
        char* label2 = new_label();
        char* label3 = new_label();

#if TRANS_PRINT_DEBUG
        // 添加调试信息
        Code* debug_code = (Code*)malloc(sizeof(Code));
        sprintf(debug_code->str, "Print in func Trans_Stmt case 2\n");
        insert_ops(debug_code, "", "", "");
        codelist_append(debug_code);
#endif
        
        Code* code1 = (Code*)malloc(sizeof(Code));
        sprintf(code1->str, "LABEL %s :\n", label1);
        insert_ops(code1, label1, "", "");
        codelist_append(code1);
        Trans_Cond(node->child[2], label2, label3);

        Code* code2 = (Code*)malloc(sizeof(Code));
        sprintf(code2->str, "LABEL %s :\n", label2);
        insert_ops(code2, label2, "", "");
        codelist_append(code2);
        Trans_Stmt(node->child[4]);

        Code* code3 = (Code*)malloc(sizeof(Code));
        sprintf(code3->str, "GOTO %s\n", label1);
        insert_ops(code3, label1, "", "");
        codelist_append(code3);

        Code* code4 = (Code*)malloc(sizeof(Code));
        sprintf(code4->str, "LABEL %s :\n", label3);
        insert_ops(code4, label3, "", "");
        codelist_append(code4);
    } else if (node->num == 5 && strcmp(node->child[0]->name, "IF") == 0 ){
        char* label1 = new_label();
        char* label2 = new_label();

        Trans_Cond(node->child[2], label1, label2);

#if TRANS_PRINT_DEBUG
        // 添加调试信息
        Code* debug_code = (Code*)malloc(sizeof(Code));
        sprintf(debug_code->str, "Print in func Trans_Stmt case 3\n");
        insert_ops(debug_code, "", "", "");
        codelist_append(debug_code);
#endif
        
        Code* code_label1 = (Code*)malloc(sizeof(Code));
        sprintf(code_label1->str, "LABEL %s :\n", label1);
        insert_ops(code_label1, label1, "", "");
        codelist_append(code_label1);

        Trans_Stmt(node->child[4]); 

        Code* code_label2 = (Code*)malloc(sizeof(Code));
        sprintf(code_label2->str, "LABEL %s :\n", label2);
        insert_ops(code_label2, label2, "", "");
        codelist_append(code_label2);
    } else if (node->num == 7) { 
        char* label1 = new_label();
        char* label2 = new_label();
        char* label3 = new_label();

        Trans_Cond(node->child[2], label1, label2);

        Code* code_label1 = (Code*)malloc(sizeof(Code));
        sprintf(code_label1->str, "LABEL %s :\n", label1);
        insert_ops(code_label1, label1, "", "");
        codelist_append(code_label1);

        Trans_Stmt(node->child[4]);  

        Code* code_goto = (Code*)malloc(sizeof(Code));
        sprintf(code_goto->str, "GOTO %s\n", label3);
        insert_ops(code_goto, label3, "", "");
        codelist_append(code_goto);

        Code* code_label2 = (Code*)malloc(sizeof(Code));
        sprintf(code_label2->str, "LABEL %s :\n", label2);
        insert_ops(code_label2, label2, "", "");
        codelist_append(code_label2);

        Trans_Stmt(node->child[6]);  

        Code* code_label3 = (Code*)malloc(sizeof(Code));
        sprintf(code_label3->str, "LABEL %s :\n", label3);
        insert_ops(code_label3, label3, "", "");
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
            insert_ops(debug_code, "", "", "");
            codelist_append(debug_code);
    #endif
            
            Code* code1 = (Code*)malloc(sizeof(Code));
            sprintf(code1->str, "%s := #%s\n", place, node->child[0]->attr);
            /*
                Insert an immediate num for op.
            */

            insert_ops(code1, place, node->child[0]->attr, "");
            codelist_append(code1);
        } else if (strcmp(node->child[0]->name, "ID") == 0) {
    #if TRANS_PRINT_DEBUG
            // 添加调试信息
            Code* debug_code = (Code*)malloc(sizeof(Code));
            sprintf(debug_code->str, "Print in func Trans_Exp case 2\n");
            insert_ops(debug_code, "", "", "");
            codelist_append(debug_code);
    #endif
#if TRANS_ARRAY_SUPPORT
            SymbolEntry* entry = lookup_symbol(node->child[0]->attr);
            assert(entry != NULL);
            if (entry->type->kind == ARRAY) {
                /* 修改点1：对数组名统一生成取地址操作 */
                char* addr_temp = new_temp();
                Trans_Exp_Addr(node, addr_temp);  // 生成数组地址
                if (place != NULL) {
                    Code* code = (Code*)malloc(sizeof(Code));
                    sprintf(code->str, "%s := %s\n", place, addr_temp);
                    insert_ops(code, place, addr_temp, "");
                    codelist_append(code);
                }
            } else {
                // 普通变量处理保持不变
                Code* code1 = (Code*)malloc(sizeof(Code));
                sprintf(code1->str, "%s := %s\n", place, entry->name);
                insert_ops(code1, place, entry->name, "");
                codelist_append(code1);
            }
#else
            Code* code1 = (Code*)malloc(sizeof(Code));
            sprintf(code1->str, "%s := %s\n", place, node->child[0]->attr);
            insert_ops(code1, place, node->child[0]->attr, "");
            codelist_append(code1);
#endif
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
        insert_ops(debug_code, "", "", "");
        codelist_append(debug_code);
#endif
        char* result = new_temp();
        Code* code_load = (Code*)malloc(sizeof(Code));
        sprintf(code_load->str, "%s := *%s\n", result, t_addr);

        char* addr_with_star = concatenate_char_with_string('*', t_addr);
        insert_ops(code_load, result, addr_with_star, "");
        free(addr_with_star);
        codelist_append(code_load);

        if (place != NULL) {
            Code* code_assign = (Code*)malloc(sizeof(Code));
            sprintf(code_assign->str, "%s := %s\n", place, result);
            insert_ops(code_assign, place, result, "");
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
        insert_ops(debug_code, "", "", "");
        codelist_append(debug_code);
#endif
        
        Code* code2 = (Code*)malloc(sizeof(Code));
        sprintf(code2->str, "%s := #0 - %s\n", place, t1);
        insert_ops(code2, place, "0", t1);
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
                insert_ops(debug_code, "", "", "");
                codelist_append(debug_code);
#endif
        // 情况1：左值是普通变量（ID节点且不是数组访问）
        if (left_exp->num == 1 && strcmp(left_exp->child[0]->name, "ID") == 0) {
            // 直接赋值给普通变量
            char* right_val = new_temp();
            Trans_Exp(node->child[2], right_val);
            Code* code_assign = (Code*)malloc(sizeof(Code));
            sprintf(code_assign->str, "%s := %s\n", left_exp->child[0]->attr, right_val);
            insert_ops(code_assign, left_exp->child[0]->attr, right_val, "");
            codelist_append(code_assign);
            if (place != NULL) {
                Code* code_place = (Code*)malloc(sizeof(Code));
                sprintf(code_place->str, "%s := %s\n", place, right_val);
                insert_ops(code_place, place, right_val, "");
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
            char* addr_with_star = concatenate_char_with_string('*', left_addr);
            insert_ops(code_store, addr_with_star, right_val, "");
            free(addr_with_star);
            codelist_append(code_store);
            if (place != NULL) {
                Code* code_assign = (Code*)malloc(sizeof(Code));
                sprintf(code_assign->str, "%s := %s\n", place, right_val);
                insert_ops(code_assign, place, right_val, "");
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
                insert_ops(debug_code, "", "", "");
                codelist_append(debug_code);
#endif
                Code* code2_1 = (Code*)malloc(sizeof(Code));   
                sprintf(code2_1->str, "%s := %s\n", node->child[0]->child[0]->attr, t1);
                insert_ops(code2_1, node->child[0]->child[0]->attr, t1, "");
                codelist_append(code2_1);
                if(place != NULL) {
                Code* code2_2 = (Code*)malloc(sizeof(Code));
                sprintf(code2_2->str, "%s := %s\n", place, node->child[0]->child[0]->attr);
                insert_ops(code2_2, place, node->child[0]->child[0]->attr, "");
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
            insert_ops(debug_code, "", "", "");
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
            
            insert_ops(code3, place, t1, t2);        
            codelist_append(code3);
        } else if (strcmp(node->child[1]->name, "RELOP") == 0 || strcmp(node->child[1]->name, "AND") == 0 || 
                   strcmp(node->child[1]->name, "OR") == 0 || strcmp(node->child[0]->name, "NOT") == 0) {
            char* label1 = new_label();
            char* label2 = new_label();

            Code* code0 = (Code*)malloc(sizeof(Code));
            sprintf(code0->str, "%s := #0\n", place);
            insert_ops(code0, place, "0", "");
            codelist_append(code0);

            Trans_Cond(node, label1, label2);

            Code* code1 = (Code*)malloc(sizeof(Code));
            sprintf(code1->str, "LABEL %s :\n", label1);
            insert_ops(code1, label1, "", "");
            codelist_append(code1);

            Code* code2 = (Code*)malloc(sizeof(Code));
            sprintf(code2->str, "%s := #1\n", place);
            insert_ops(code2, place, "1", "");
            codelist_append(code2);

            Code* code3 = (Code*)malloc(sizeof(Code));
            sprintf(code3->str, "LABEL %s :\n", label2);
            insert_ops(code3, label2, "", "");
            codelist_append(code3);
        } else if (strcmp(node->child[0]->name, "ID") == 0) {
            if (strcmp(node->child[0]->attr, "read") == 0) {
#if TRANS_PRINT_DEBUG
                // 添加调试信息
                Code* debug_code = (Code*)malloc(sizeof(Code));
                sprintf(debug_code->str, "Print in func Trans_Exp case 6\n");
                insert_ops(debug_code, "", "", "");
                codelist_append(debug_code);
#endif
                
                Code* code1 = (Code*)malloc(sizeof(Code));
                sprintf(code1->str, "READ %s\n", place);
                insert_ops(code1, place, "", "");
                codelist_append(code1);
            } else {
#if TRANS_PRINT_DEBUG
                // 添加调试信息
                Code* debug_code = (Code*)malloc(sizeof(Code));
                sprintf(debug_code->str, "Print in func Trans_Exp case 7\n");
                insert_ops(debug_code, "", "", "");
                codelist_append(debug_code);
#endif
                
                Code* code2 = (Code*)malloc(sizeof(Code));
                sprintf(code2->str, "%s := CALL %s\n", place, node->child[0]->attr);
                insert_ops(code2, place, node->child[0]->attr, "");
                codelist_append(code2);
            } 
        }
    } else if (node->num == 4 && strcmp(node->child[0]->name, "ID") == 0) { // ID LP Args RP
        Arg* arg_list_head = NULL;
        Arg* arg_list_tail = NULL;
        Trans_Args(node->child[2], &arg_list_head, &arg_list_tail);

        if (strcmp(node->child[0]->attr, "write") == 0) {
#if TRANS_PRINT_DEBUG
            // 添加调试信息
            Code* debug_code = (Code*)malloc(sizeof(Code));
            sprintf(debug_code->str, "Print in func Trans_Exp case 8\n");
            insert_ops(debug_code, "", "", "");
            codelist_append(debug_code);
#endif

            Code* code1 = (Code*)malloc(sizeof(Code));
            sprintf(code1->str, "WRITE %s\n", arg_list_head->name);
            insert_ops(code1, arg_list_head->name, "", "");
            codelist_append(code1);
            if(place != NULL) {
                Code* code2 = (Code*)malloc(sizeof(Code));
                sprintf(code2->str, "%s := #0\n", place);
                insert_ops(code2, place, "0", "");
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
                insert_ops(debug_code, "", "", "");
                codelist_append(debug_code);
#endif
                Code* code_arg = (Code*)malloc(sizeof(Code));
                sprintf(code_arg->str, "ARG %s\n", cur->name);
                insert_ops(code_arg, cur->name, "", "");
                codelist_append(code_arg);
                cur = cur->next;
            }

            Code* code_call = (Code*)malloc(sizeof(Code));
            sprintf(code_call->str, "%s := CALL %s\n", place, node->child[0]->attr);
            insert_ops(code_call, place, node->child[0]->attr, "");
            codelist_append(code_call);
        }
    }
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
    insert_ops(debug_code, "", "", "");
    codelist_append(debug_code);
#endif     

    char* func_name = node->child[0]->attr;
    Code* func_code = (Code*)malloc(sizeof(Code));
    sprintf(func_code->str, "FUNCTION %s :\n", func_name);
    insert_ops(func_code, func_name, "", "");
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





#if TRANS_ARRAY_SUPPORT
// 数组支持专用代码块

// 新增辅助函数：计算数组总大小（字节数）
int get_array_size(Type type) {
    int size = 1;
    while (type->kind == ARRAY) {
        size *= type->u.array.size;
        type = type->u.array.elem;
    }
    return size * 4; // 假设int类型宽度为4
}

void Trans_Exp_Addr(Node* node, char* place) {
    // Case 1：简单变量（ID）
    if (node->num == 1 && strcmp(node->child[0]->name, "ID") == 0) {
        SymbolEntry* entry = lookup_symbol(node->child[0]->attr);
        assert(entry != NULL);
        Code* code = (Code*)malloc(sizeof(Code));
        if (entry->type->kind == ARRAY) {
            if (entry->is_param) {
                // 参数数组直接使用名称作为地址
                sprintf(code->str, "%s := %s\n", place, entry->name);
                insert_ops(code, place, entry->name, "");
            } else {
                // 非参数数组生成取地址指令
                sprintf(code->str, "%s := &%s\n", place, entry->name);
                char* name_with_and = concatenate_char_with_string('&', entry->name);
                insert_ops(code, place, name_with_and, "");
                free(name_with_and);
            }
        } else {
            // 普通变量，取地址
            sprintf(code->str, "%s := &%s\n", place, entry->name);
            char* name_with_and = concatenate_char_with_string('&', entry->name);
            insert_ops(code, place, name_with_and, "");
            free(name_with_and);
        }
        codelist_append(code);
    }
    // Case 2：数组访问（Exp LB Exp RB）
    else if (node->num == 4 && strcmp(node->child[1]->name, "LB") == 0) {
        // 处理多维数组访问
        Node* base_node = node->child[0];
        char* t_base = new_temp();
        // 递归获取基地址
        Trans_Exp_Addr(base_node, t_base);
        SymbolEntry* entry = lookup_symbol(get_var_name(node->child[0]));
        
        assert(entry);
        assert(entry->type->kind == ARRAY);

        Type current_type = entry->type;
        Node* index_node = node;
        char* total_offset = new_temp();
        sprintf(total_offset, "#0"); // 初始偏移为0
        
        // 遍历所有维度
        while (index_node->num == 4 && strcmp(index_node->child[1]->name, "LB") == 0) {
            // 计算当前维度的索引
            char* t_index = new_temp();
            Trans_Exp(index_node->child[2], t_index);
            
            // 计算后续维度的乘积（元素个数）
            int element_count = 1;
            Type temp_type = current_type->u.array.elem;
            while (temp_type->kind == ARRAY) {
                element_count *= temp_type->u.array.size;
                temp_type = temp_type->u.array.elem;
            }
            
            // 生成中间代码：offset += index * stride（stride = element_count * 4）
            char* t_stride = new_temp();
            Code* code_stride = (Code*)malloc(sizeof(Code));
            sprintf(code_stride->str, "%s := #%d\n", t_stride, element_count * 4);
            char str[MAX_CODE_LENGTH];
            snprintf(str, sizeof(str), "%d", element_count * 4);
            insert_ops(code_stride, t_stride, str, "");
            codelist_append(code_stride);
            
            char* t_mul = new_temp();
            Code* code_mul = (Code*)malloc(sizeof(Code));
            sprintf(code_mul->str, "%s := %s * %s\n", t_mul, t_index, t_stride);
            insert_ops(code_mul, t_mul, t_index, t_stride);
            codelist_append(code_mul);
            
            char* new_total = new_temp();
            Code* code_add = (Code*)malloc(sizeof(Code));
            sprintf(code_add->str, "%s := %s + %s\n", new_total, total_offset, t_mul);
            insert_ops(code_add, new_total, total_offset, t_mul);
            codelist_append(code_add);
            
            total_offset = new_total;
            current_type = current_type->u.array.elem;
            index_node = index_node->child[0];
        }
        
        // 最终地址计算（不需要再乘4）
        Code* code_addr = (Code*)malloc(sizeof(Code));
        sprintf(code_addr->str, "%s := %s + %s\n", place, t_base, total_offset);
        insert_ops(code_addr, place, t_base, total_offset);
        codelist_append(code_addr);
    }
    else {
        assert(0); // 不支持的左值形式
    }
}

void Trans_VarDec(Node* node) {
#if TRANS_PRINT_DEBUG
    printf("Enter Trans_VarDec\n");
#endif
    if (node->num == 4 && strcmp(node->child[1]->name, "LB") == 0) {
        // 数组声明处理（类型检查假设已在语义分析完成）
#if TRANS_ARRAY_SUPPORT
        // 递归解析所有维度
        Node* base = node;
        while (base->child[0]->num == 4) 
            base = base->child[0];
        char* array_name = base->child[0]->child[0]->attr;
        SymbolEntry* entry = lookup_symbol(array_name);
        if (entry && !entry->is_param) { 
            // 计算总字节数 = 各维度乘积 * 4
            int total_size = 1;
            Type current_type = entry->type;
            while (current_type->kind == ARRAY) {
                total_size *= current_type->u.array.size;
                current_type = current_type->u.array.elem;
            }
            total_size *= 4;

            // 生成DEC指令
            Code* dec_code = (Code*)malloc(sizeof(Code));
            sprintf(dec_code->str, "DEC %s %d\n", array_name, total_size);
            char str[MAX_CODE_LENGTH];
            snprintf(str, sizeof(str), "%d", total_size);
            insert_ops(dec_code, array_name, str, "");
            codelist_append(dec_code);
        }
#endif
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
                char* param_name = var_dec->child[0]->child[0]->attr;
                
                SymbolEntry* entry = lookup_symbol(param_name);
                assert(entry != NULL);
                assert(entry->type->kind == ARRAY);
                entry->is_param = 1; // 标记为参数
                
                // 生成PARAM指令
                Code* param_code = (Code*)malloc(sizeof(Code));
                sprintf(param_code->str, "PARAM %s\n", param_name);
                insert_ops(param_code, param_name, "", "");
                codelist_append(param_code);
            }
            // 处理普通变量
            else if (var_dec->num == 1 && strcmp(var_dec->child[0]->name, "ID") == 0) {
                char* param_name = var_dec->child[0]->attr;
                SymbolEntry* entry = lookup_symbol(param_name);
                assert(entry != NULL);
                entry->is_param = 1;
                
                Code* param_code = (Code*)malloc(sizeof(Code));
                sprintf(param_code->str, "PARAM %s\n", param_name);
                insert_ops(param_code, param_name, "", "");
                codelist_append(param_code);
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
                insert_ops(param_code, param_name, "", "");
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


void print_code_codetype(Code* code){
    for(int i = 0; i < 3; i++){
        switch(code->ops[i].optype){
            case OP_EMPTY:
                printf("OP_EMPTY  ");
                break;
            case OP_CONST:
                printf("OP_CONST  ");
                break;
            case OP_ADDR:
                printf("OP_ADDR  ");
                break;
            case OP_DEREF:
                printf("OP_DEREF  ");
                break;
            case OP_TEMP:
                printf("OP_TEMP  ");
                break;
            case OP_LABEL:
                printf("OP_LABEL  ");
                break;
            default:
                assert(0);
        }
    }


switch (code->codetype) {
    case CODE_UNKNOWN:
        printf("code: CODE_UNKNOWN\n");
        break;
    case CODE_READ:
        printf("code: CODE_READ\n");
        break;
    case CODE_WRITE:
        printf("code: CODE_WRITE\n");
        break;
    case CODE_LABEL:
        printf("code: CODE_LABEL\n");
        break;
    case CODE_ADD:
        printf("code: CODE_ADD\n");
        break;
    case CODE_SUB:
        printf("code: CODE_SUB\n");
        break;
    case CODE_MUL:
        printf("code: CODE_MUL\n");
        break;
    case CODE_DIV:
        printf("code: CODE_DIV\n");
        break;
    case CODE_SIMPLE_ASSIGNOP:
        printf("code: CODE_SIMPLE_ASSIGNOP\n");
        break;
    case CODE_ARG:
        printf("code: CODE_ARG\n");
        break;
    case CODE_CALL:
        printf("code: CODE_CALL\n");
        break;
    case CODE_GOTO:
        printf("code: CODE_GOTO\n");
        break;
    case CODE_RETURN:
        printf("code: CODE_RETURN\n");
        break;
    case CODE_IFGOTO:
        printf("code: CODE_IFGOTO\n");
        break;
    case CODE_FUNCTION:
        printf("code: CODE_FUNCTION\n");
        break;
    case CODE_PARAM:
        printf("code: CODE_PARAM\n");
        break;
    case CODE_DEC:
        printf("code: CODE_DEC\n");
        break;
    case CODE_HANDLED:
        printf("code: CODE_HANDLED\n");
        break;
    case CODE_ASSIGN_DEREF:
        printf("code: CODE_ASSIGN_DEREF\n");
        break;
    case CODE_ASSIGN_ADDR:
        printf("code: CODE_ASSIGN_ADDR\n");
        break;
    default:
        printf("Error: Unknown CodeType\n");
        break;
    }
}



char* concatenate_char_with_string(char c, const char* name) {
    if (name == NULL) {
        // 如果 name 为 NULL，返回 NULL
        return NULL;
    }

    // 计算新字符串的长度（包括字符 c 和字符串 name，以及结尾的 '\0'）
    size_t new_length = strlen(name) + 2; // +1 用于字符 c，+1 用于 '\0'

    // 动态分配内存
    char* new_name = (char*)malloc(new_length * sizeof(char));
    if (new_name == NULL) {
        // 如果内存分配失败，返回 NULL
        return NULL;
    }

    // 将字符 c 和字符串 name 拼接到 new_name 中
    new_name[0] = c; // 放置字符 c
    strcpy(new_name + 1, name); // 拷贝字符串 name 到 new_name 的剩余部分

    return new_name;
}



// 自定义 strdup 函数
char* my_strdup(const char* s) {
    if (s == NULL) {
        return NULL; // 如果输入为 NULL，直接返回 NULL
    }

    // 计算字符串长度（包括结尾的 '\0'）
    size_t len = strlen(s) + 1;

    // 动态分配内存
    char* copy = (char*)malloc(len);
    if (copy == NULL) {
        return NULL; // 如果分配失败，返回 NULL
    }

    // 复制字符串
    memcpy(copy, s, len);

    return copy;
}