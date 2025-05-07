#include "trans_mips.h"
#include "lib.h"

int local_offset;   // The offset of the variables in a stack-frame.
FrameVar frameVarHead;

void init_regs(){
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
        regs[i].value = NULL;
    }
}

unsigned int get_free_reg(){
    /*
        Need to be finished.
    */
    return 0;
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
        if( op.optype == OP_ADDR || 
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


void Trans_MIPS(char* filename){
    Code* code = codelist.head->next;
    FILE *file = fopen(filename, "w");
    while(code) {
        Trans_MIPS_SingleLineCode(code, file);
        code = code->next;
    }
    fclose(file);
}


void Trans_MIPS_SingleLineCode(Code* code, FILE* file){
    assert(code);
    switch(code->codetype){
        case CODE_READ:
        //    Trans_MIPS_read(code);
            break;
        case CODE_WRITE:
        //    Trans_MIPS_write(code);
            break;
        case CODE_FUNCTION:
            Trans_MIPS_function(code, file);
            break;
        case CODE_HANDLED:
            break;
        default:
        //    printf("ERROR: Code type unknown\n");
        //    assert(0);
            break;
    }
}


void Trans_MIPS_read(Code* code){}
void Trans_MIPS_write(Code* code){}
void Trans_MIPS_label(Code* code){}
void Trans_MIPS_add(Code* code){}
void Trans_MIPS_sub(Code* code){}
void Trans_MIPS_mul(Code* code){}
void Trans_MIPS_div(Code* code){}
void Trans_MIPS_assignop(Code* code){}
void Trans_MIPS_arg_call(Code* code){}
void Trans_MIPS_goto(Code* code){}
void Trans_MIPS_return(Code* code){}
void Trans_MIPS_ifgoto(Code* code){}

void Trans_MIPS_function(Code* code, FILE* file){
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