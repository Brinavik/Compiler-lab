#include "semantic_analysis_error.h"

void semErrOutput(unsigned int errCode, unsigned int lineNum){
    switch(errCode){
        case NOT_DEFINE_VAR:
            printf("Error type 1 at line %u: 1\n", lineNum);
            break;
        case(NOT_DEFINE_FUNC):
            printf("Error type 2 at line %u: 2\n", lineNum);
            break;
        case(DEFINE_VAR_MULTIPLY):
            printf("Error type 3 at line %u: 3\n", lineNum);
            break;
        case(DEFINE_FUNC_MULTIPLY):
            printf("Error type 4 at line %u: 4\n", lineNum);
            break;
        case(NOT_MATCH_ASSIGNOP):
            printf("Error type 5 at line %u: 5\n", lineNum);
            break;
        case(NOT_LEGAL_LVAL):
            printf("Error type 6 at line %u: 6\n", lineNum);
            break;
        case(NOT_MATCH_OPERATOR):
            printf("Error type 7 at line %u: 7\n", lineNum);
            break;
        case(NOT_MATCH_RETURN):
            printf("Error type 8 at line %u: 8\n", lineNum);
            break;               
        case(NOT_MATCH_FUNCPARA):
            printf("Error type 9 at line %u: 9\n", lineNum);
            break;    
        case(NOT_ARR_LB):
            printf("Error type 10 at line %u: 10\n", lineNum);
            break;    
        case(NOT_FUNC_LC):
            printf("Error type 11 at line %u: 11\n", lineNum);
            break;    
        case(NOT_INT_ACCESS_ARR):
            printf("Error type 12 at line %u: 12\n", lineNum);
            break;    
        case(NOT_STRUCT_DOT):
            printf("Error type 13 at line %u: 13\n", lineNum);
            break;    
        case(NOT_DEFINE_FIELD):
            printf("Error type 14 at line %u: 14\n", lineNum);
            break;    
        case(DEFINE_FIELD_MULTIPLY):
            printf("Error type 15 at line %u: 15\n", lineNum);
            break;    
        case(DEFINE_STRUCT_MULTIPLY):
            printf("Error type 16 at line %u: 16\n", lineNum);
            break;    
        case(NOT_DEFINE_STRUCT):
            printf("Error type 17 at line %u: 17\n", lineNum);
            break;    
        }
}



int is_lvalue(Node* node) {
    if (!node) 
        return 0;
    switch (node->num) {
        // ID 
        case 1:
            if(strcmp(node->child[0]->name, "ID") == 0)
                return 1;
        case 3:
            // Exp DOT ID
            if(strcmp(node->child[1]->name, "DOT") == 0  &&
            strcmp(node->child[2]->name, "ID") == 0)
                return 1;
            // LP Exp RP
            else if (strcmp(node->child[0]->name, "LP") == 0 && 
            strcmp(node->child[2]->name, "DOT") == 0)
                return is_lvalue(node->child[1]);
        // Exp LB Exp RB 
        case 4: 
            if(strcmp(node->child[1]->name, "LB") == 0  &&
            strcmp(node->child[3]->name, "RB") == 0)
                return 1;

        default:
            return 0;
    }
    return 0;
}