#include "trans_mips.h"
#include "lib.h"

FILE* file;
int local_offset;   // The offset of the variables in a stack-frame.
FrameVar frameVarHead;


void init_MIPS() {
    // init the registers.
    strcpy(regs[0].name,"$zero");
    strcpy(regs[1].name,"$at");
    strcpy(regs[2].name,"$v0");
    strcpy(regs[3].name,"$v1");
    strcpy(regs[4].name,"$a0");
    strcpy(regs[5].name,"$a1");
    strcpy(regs[6].name,"$a2");
    strcpy(regs[7].name,"$a3");
    strcpy(regs[8].name,"$t0");
    strcpy(regs[9].name,"$t1");
    strcpy(regs[10].name,"$t2");
    strcpy(regs[11].name,"$t3");
    strcpy(regs[12].name,"$t4");
    strcpy(regs[13].name,"$t5");
    strcpy(regs[14].name,"$t6");
    strcpy(regs[15].name,"$t7");
    strcpy(regs[16].name,"$s0");
    strcpy(regs[17].name,"$s1");
    strcpy(regs[18].name,"$s2");
    strcpy(regs[19].name,"$s3");
    strcpy(regs[20].name,"$s4");
    strcpy(regs[21].name,"$s5");
    strcpy(regs[22].name,"$s6");
    strcpy(regs[23].name,"$s7");
    strcpy(regs[24].name,"$t8");
    strcpy(regs[25].name,"$t9");
    strcpy(regs[26].name,"$k0");
    strcpy(regs[27].name,"$k1");
    strcpy(regs[28].name,"$gp");
    strcpy(regs[29].name,"$sp");
    strcpy(regs[30].name,"$fp");
    strcpy(regs[31].name,"$ra");
    for(int i=0;i<32;i++) {
        regs[i].state=REGISTER_FREE;
        regs[i].var = NULL;
    }


    // init the MIPS program.
    fprintf(file,".data\n");
    fprintf(file,"_prompt: .asciiz \"Enter an integer:\"\n");
    fprintf(file,"_ret: .asciiz \"\\n\"\n");
    fprintf(file,".globl main\n");
    fprintf(file,".text\n");
    fprintf(file,"read:\n");
    fprintf(file,"li $v0, 4\n");
    fprintf(file,"la $a0, _prompt\n");
    fprintf(file,"syscall\n");
    fprintf(file,"li $v0, 5\n");
    fprintf(file,"syscall\n");
    fprintf(file,"jr $ra\n");

    fprintf(file,"\n");
    fprintf(file,"write:\n");
    fprintf(file,"li $v0, 1\n");
    fprintf(file,"syscall\n");
    fprintf(file,"li $v0, 4\n");
    fprintf(file,"la $a0, _ret\n");
    fprintf(file,"syscall\n");
    fprintf(file,"move $v0, $0\n");
    fprintf(file,"jr $ra\n\n");
}


unsigned int get_free_reg(Op op){
    if(op.optype == OP_EMPTY)
    // it should not appear!
        assert(0);
#if TRANS_MIPS_DEBUG
    printf("get-reg-a\n");
    printf("optype %d\n", op.optype);
#endif
    if(op.optype == OP_CONST){
        unsigned int j;
        for(j = 8; j <= 15; j++){
            if(regs[j].state == REGISTER_FREE){
                regs[j].state = REGISTER_OCCUPIED;
                regs[j].var = NULL;
                fprintf(file, "li %s, %s\n", regs[j].name, op.name);
                return j;
            }
        }
    }
#if TRANS_MIPS_DEBUG
    printf("get-reg-b\n");
#endif
    unsigned int i;
    for(i = 8; i <= 15; i++){
        if(regs[i].state == REGISTER_FREE){
            regs[i].state = REGISTER_OCCUPIED;
            FrameVar temp = find_frame_var(op);
            assert(temp);
            temp->reg = i;
            regs[i].var = temp;

            if(op.optype == OP_ADDR) {
                fprintf(file, "addi %s, $fp, %d\n", regs[i].name, temp->offset);
            } else if(op.optype == OP_DEREF) {
                fprintf(file,"lw %s, %d($fp)\n",regs[i].name,temp->offset);
                fprintf(file,"lw %s, 0(%s)\n",regs[i].name,regs[i].name);
            } else {
                fprintf(file,"lw %s, %d($fp)\n",regs[i].name,temp->offset);
            }
            break;
        }
    }
    if(i == 16){
        printf("No free register left ?\n");
        assert(0);
    }
    return i;
}


/*
frame_var这里我用链表实现，虽然效率稍低，但是胜在简单。也可以像符号表那样用哈希表结构.
*/
void insert_frame_var(FrameVar temp){
    temp->next = frameVarHead;
    frameVarHead = temp;
}

FrameVar find_frame_var(Op op){
    if(op.optype == OP_CONST){
        assert(0);
    }
    FrameVar tmp = frameVarHead;
    while(tmp){
        if(strcmp(tmp->name, op.name) == 0)
            break;
        tmp = tmp->next;
    }
    return tmp;
}

void insert_frame_var_for_op(Op op){
        if( op.optype == OP_CONST || 
            op.optype == OP_EMPTY)
            return;
    #if TRANS_MIPS_DEBUG
        printf("%s\n", op.name);
    #endif
        FrameVar temp = find_frame_var(op);
//第一次出现这个变量，需要新建一个FrameVar_，并且插入链表中
        if(temp == NULL) { 
            local_offset += 4;
            FrameVar new_var = (FrameVar)calloc(1, sizeof(FrameVar_));
            strcpy(new_var->name, op.name);
            new_var->next = NULL;
            new_var->offset= -local_offset;
            insert_frame_var(new_var);
        }    
}

void load_memory(unsigned int reg){
    assert(regs[reg].name);
#if TRANS_MIPS_DEBUG
    printf("load memory of %s\n", regs[reg].name);
#endif
    assert(regs[reg].var);
    int offset=regs[reg].var->offset;
    fprintf(file,"sw %s, %d($fp)\n",regs[reg].name,offset);
    //Free the regs
    for(int i=8;i<16;i++){
        if(regs[i].state == REGISTER_OCCUPIED)
            regs[i].state = REGISTER_FREE;
    }
}






void Trans_MIPS(char* filename){
    Code* code = codelist.head->next;
    file = fopen(filename, "w");
    init_MIPS();
    while(code) {
        Trans_MIPS_SingleLineCode(code);
        code = code->next;
    }
    fclose(file);
}


void Trans_MIPS_SingleLineCode(Code* code){
    assert(code);
    switch(code->codetype){
        case CODE_READ:
    #if TRANS_MIPS_DEBUG
        printf("Trans CODE_READ\n");
    #endif
            Trans_MIPS_read(code);
            break;

        case CODE_WRITE:
    #if TRANS_MIPS_DEBUG
        printf("Trans CODE_WRITE\n");
    #endif
            Trans_MIPS_write(code);
            break;

        case CODE_ADD:
    #if TRANS_MIPS_DEBUG
        printf("Trans CODE_ADD\n");
    #endif
            Trans_MIPS_add(code);
            break;

        case CODE_SUB:
    #if TRANS_MIPS_DEBUG
        printf("Trans CODE_SUB\n");
    #endif
            Trans_MIPS_sub(code);
            break;

        case CODE_MUL:
    #if TRANS_MIPS_DEBUG
        printf("Trans CODE_MUL\n");
    #endif
            Trans_MIPS_mul(code);
            break;

        case CODE_DIV:
    #if TRANS_MIPS_DEBUG
        printf("Trans CODE_DIV\n");
    #endif
            Trans_MIPS_div(code);
            break;
        
        case CODE_ASSIGN_ADDR:
        case CODE_ASSIGN_DEREF:
        case CODE_SIMPLE_ASSIGNOP:
    #if TRANS_MIPS_DEBUG
        printf("Trans CODE_ASSIGN_FAMILY\n");
    #endif
        Trans_MIPS_assignop(code);
        break;

        case CODE_ARG:
        case CODE_CALL:
    #if TRANS_MIPS_DEBUG
        printf("Trans CODE_ARG/CALL\n");
    #endif
        Trans_MIPS_arg_call(code);
        break;

        case CODE_GOTO:
    #if TRANS_MIPS_DEBUG
        printf("Trans CODE_ARG/CALL\n");
    #endif
        Trans_MIPS_goto(code);
        break;

        case CODE_IFGOTO:
    #if TRANS_MIPS_DEBUG
        printf("Trans CODE_ARG/CALL\n");
    #endif
        Trans_MIPS_ifgoto(code);
        break;

        case CODE_FUNCTION:
    #if TRANS_MIPS_DEBUG
        printf("Trans CODE_FUNCTION\n");
    #endif
        Trans_MIPS_function(code);
        break;
        
        case CODE_LABEL:
    #if TRANS_MIPS_DEBUG
        printf("Trans CODE_LABEL\n");
    #endif
        Trans_MIPS_label(code);
        break;
        
        case CODE_HANDLED:
    #if TRANS_MIPS_DEBUG
        printf("Trans CODE_HANDLED\n");
    #endif
            break;

        default:
            printf("ERROR: Code type unknown\n");
            assert(0);
            break;
    }
}


void Trans_MIPS_read(Code* code) {
    fprintf(file,"addi $sp, $sp, -4\n");
    fprintf(file,"sw $ra, 0($sp)\n");
    fprintf(file,"jal read\n");
    fprintf(file,"lw $ra, 0($sp)\n");
    fprintf(file,"addi $sp, $sp, 4\n");
    unsigned int reg=get_free_reg(code->ops[0]);
    fprintf(file,"move %s, $v0\n",regs[reg].name);
    load_memory(reg);
}

void Trans_MIPS_write(Code* code) {
    unsigned int reg=get_free_reg(code->ops[0]);
    fprintf(file,"move $a0, %s\n",regs[reg].name);
    fprintf(file,"addi $sp, $sp, -4\n");
    fprintf(file,"sw $ra, 0($sp)\n");
    fprintf(file,"jal write\n");
    fprintf(file,"lw $ra, 0($sp)\n");
    fprintf(file,"addi $sp, $sp, 4\n");
    for(int i=8;i<16;i++){
        if(regs[i].state == REGISTER_OCCUPIED)
            regs[i].state = REGISTER_FREE;
    }
}

void Trans_MIPS_label(Code* code) {
    fprintf(file, "%s:\n", code->ops[0].name);
}

void Trans_MIPS_add(Code* code) {
        int res=get_free_reg(code->ops[0]);
        int op1=get_free_reg(code->ops[1]);
        int op2=get_free_reg(code->ops[2]);
        fprintf(file,"add %s, %s, %s\n",regs[res].name,regs[op1].name,regs[op2].name);
        load_memory(res);
}

void Trans_MIPS_sub(Code* code) {
    int res=get_free_reg(code->ops[0]);
    int op1=get_free_reg(code->ops[1]);
    int op2=get_free_reg(code->ops[2]);
    fprintf(file,"sub %s, %s, %s\n",regs[res].name,regs[op1].name,regs[op2].name);
    load_memory(res);    
}

void Trans_MIPS_mul(Code* code) {
    int res=get_free_reg(code->ops[0]);
    int op1=get_free_reg(code->ops[1]);
    int op2=get_free_reg(code->ops[2]);
    fprintf(file,"mul %s, %s, %s\n",regs[res].name,regs[op1].name,regs[op2].name);
    load_memory(res);
}

void Trans_MIPS_div(Code* code) {
    int res=get_free_reg(code->ops[0]);
    int op1=get_free_reg(code->ops[1]);
    int op2=get_free_reg(code->ops[2]);
    fprintf(file,"div %s, %s, %s\n",regs[res].name,regs[op1].name,regs[op2].name);
    load_memory(res);
}

void Trans_MIPS_assignop(Code* code){
    unsigned int rreg = get_free_reg(code->ops[1]);
    if(code->ops[0].optype != OP_DEREF) {
        unsigned int lreg = get_free_reg(code->ops[0]);
        fprintf(file, "move %s, %s\n", regs[lreg].name, regs[rreg].name);
        load_memory(lreg);
        return;
    }

    FrameVar lvar = find_frame_var(code->ops[0]);
    assert(lvar);
    int i;
    for(i = 8; i <= 15; i++){
        if(regs[i].state == REGISTER_FREE)
            break;
    }
    assert(i != 16);
    regs[i].state = REGISTER_OCCUPIED;
    fprintf(file, "lw %s, %d($fp)\n", regs[i].name, lvar->offset);
    fprintf(file, "sw %s, 0(%s)\n", regs[rreg].name,regs[i].name);
    regs[i].state = REGISTER_FREE;
    regs[rreg].state = REGISTER_FREE;
}

void Trans_MIPS_arg_call(Code* code){
    int num = 0;
    // ARG
    if(code->codetype == CODE_ARG) {
        while(code->codetype == CODE_ARG) {
            num += 1;
            fprintf(file,"addi $sp, $sp, -4\n");
            int reg = get_free_reg(code->ops[0]);
            fprintf(file,"sw %s, 0($sp)\n",regs[reg].name);
            regs[reg].state = REGISTER_FREE;
            code->codetype = CODE_HANDLED;
            code = code->next;
        }
    }
    // CALL
    assert(code->codetype == CODE_CALL);
    fprintf(file, "jal %s\n", code->ops[1].name);
    fprintf(file, "addi, $sp, $sp, %d\n", num * 4);
    int reg = get_free_reg(code->ops[0]);
    fprintf(file, "move %s, $v0\n", regs[reg].name);
    load_memory(reg);
    code->codetype = CODE_HANDLED;
}

void Trans_MIPS_goto(Code* code) {
    fprintf(file,"j %s\n",code->ops[0].name);
}

void Trans_MIPS_return(Code* code) {
    fprintf(file,"lw $ra, 4($fp)\n");
    fprintf(file,"addi $sp, $fp, 8\n");
    unsigned int reg = get_free_reg(code->ops[0]);
    //RETURN 
    fprintf(file,"lw $fp, 0($fp)\n");
    fprintf(file,"move $v0, %s\n",regs[reg].name);
    fprintf(file,"jr $ra\n");
    for(int i=8;i<16;i++){
        if(regs[i].state == REGISTER_OCCUPIED)
            regs[i].state = REGISTER_FREE;
    }
}

void Trans_MIPS_ifgoto(Code* code) {
    char relop[4];
    extract_relop_from_ifgoto(code, relop);
    int lreg = get_free_reg(code->ops[0]);
    int rreg = get_free_reg(code->ops[1]);
    if(strcmp(relop,"==")==0) {
            fprintf(file,"beq %s, %s, %s\n",regs[lreg].name,regs[rreg].name,code->ops[2].name);
    }
    else if(strcmp(relop,"!=")==0) {
            fprintf(file,"bne %s, %s, %s\n",regs[lreg].name,regs[rreg].name,code->ops[2].name);
    }
    else if(strcmp(relop,">")==0) {
            fprintf(file,"bgt %s, %s, %s\n",regs[lreg].name,regs[rreg].name,code->ops[2].name);
    }
    else if(strcmp(relop,"<")==0) {
            fprintf(file,"blt %s, %s, %s\n",regs[lreg].name,regs[rreg].name,code->ops[2].name);
    }
    else if(strcmp(relop,">=")==0) {
            fprintf(file,"bge %s, %s, %s\n",regs[lreg].name,regs[rreg].name,code->ops[2].name);
    }
    else if(strcmp(relop,"<=")==0) {
            fprintf(file,"ble %s, %s, %s\n",regs[lreg].name,regs[rreg].name,code->ops[2].name);
    }
    regs[lreg].state = REGISTER_FREE;
    regs[rreg].state = REGISTER_FREE;
}

void Trans_MIPS_function(Code* code){
    fprintf(file, "\n%s:\n", code->ops[0].name);
    fprintf(file,"addi $sp, $sp, -8\n");
    fprintf(file,"sw $fp, 0($sp)\n");
    fprintf(file,"sw $ra, 4($sp)\n");
    fprintf(file,"move $fp, $sp\n");
    local_offset = 0;
    int cnt = 0;

    code = code->next;
// 向栈帧中插入函数的参数
    while(code->codetype == CODE_PARAM){
        code->codetype = CODE_HANDLED;
        FrameVar param = (FrameVar)calloc(1, sizeof(FrameVar_));
        strcpy(param->name, code->ops[0].name);
        param->offset = 8 + cnt*4;
        insert_frame_var(param);
        cnt += 1;
        code = code->next;
    }

// 向栈帧插入函数的临时变量
    while(code && code->codetype != CODE_FUNCTION){
        switch(code->codetype){
            case CODE_ADD:
            case CODE_SUB:
            case CODE_MUL:
            case CODE_DIV:
            #if TRANS_MIPS_DEBUG  
                printf("func-case 0\n");  
            #endif  
                for(int i = 0; i < 3; i++)
                    insert_frame_var_for_op(code->ops[i]);
                break;

            case CODE_ASSIGN_ADDR:
            case CODE_ASSIGN_DEREF:
            case CODE_SIMPLE_ASSIGNOP:
            #if TRANS_MIPS_DEBUG  
                printf("func-case 1\n");  
            #endif  
                for(int i = 0; i < 2; i++)
                    insert_frame_var_for_op(code->ops[i]);
                break;

            case CODE_DEC:
            #if TRANS_MIPS_DEBUG  
                printf("func-case 2\n");  
            #endif  
                local_offset += atoi(code->ops[1].name);
                FrameVar arr = (FrameVar)calloc(1, sizeof(FrameVar_));
                strcpy(arr->name, code->ops[0].name);
                arr->offset = (-1)*local_offset;
                insert_frame_var(arr);
                break;
            
            case CODE_CALL: 
            // not sure.
            #if TRANS_MIPS_DEBUG  
                printf("func-case 3\n");  
            #endif  
                insert_frame_var_for_op(code->ops[0]);
                break;
            
            case CODE_ARG:
            #if TRANS_MIPS_DEBUG  
                printf("func-case 4\n");  
            #endif  
                insert_frame_var_for_op(code->ops[0]);
                break;
            
            case CODE_IFGOTO:
            // not sure.
            #if TRANS_MIPS_DEBUG  
                printf("func-case 5\n");  
            #endif  
                for(int i = 0; i < 2; i++)
                    insert_frame_var_for_op(code->ops[i]);
                break;
            
            case CODE_READ:
            case CODE_WRITE:
            #if TRANS_MIPS_DEBUG  
                printf("func-case 6\n");  
            #endif  
                insert_frame_var_for_op(code->ops[0]);
                break;
            
            case CODE_GOTO:
            case CODE_LABEL:
            case CODE_RETURN:
            #if TRANS_MIPS_DEBUG  
                printf("func-case 7\n");  
            #endif  
                break;

            default:
                printf("What kind of CODE is this: %d\n", code->codetype);
                assert(0);
        }
        code = code->next;
    }
    fprintf(file,"addi $sp, $sp, %d\n",(-1)*local_offset);
    for(int i = 8; i < 16; i++) {
        if(regs[i].state == REGISTER_OCCUPIED)
            regs[i].state = REGISTER_FREE;
    }
}


void extract_relop_from_ifgoto(Code* code, char* dest){
    assert(code->codetype == CODE_IFGOTO);
    char* input = code->str;
    for(int i = 0; input[i] != '\0'; i++){
        char c = input[i];
        if (c == '>' || c == '<' || c == '!' || c == '=') {
            // 检查可能的双字符relop
            if (input[i+1] != '\0') { // 确保i+1有效
                char next_c = input[i+1];
                if ((c == '>' && next_c == '=') ||
                    (c == '<' && next_c == '=') ||
                    (c == '=' && next_c == '=') ||
                    (c == '!' && next_c == '=')) {
                    static char relop[3];
                    relop[0] = c;
                    relop[1] = next_c;
                    relop[2] = '\0';
                    strcpy(dest, relop);
                }
            }
            // 处理单字符的>或<
            if (c == '>' || c == '<') {
                static char relop[2];
                relop[0] = c;
                relop[1] = '\0';
                strcpy(dest, relop);
            }
        }
    }
}