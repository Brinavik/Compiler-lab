#ifndef SEMANTIC_ANALYSIS_ERROR_H
#define SEMANTIC_ANALYSIS_ERROR_H

#include "node.h"
#include "type.h"
#include <stdio.h>

#define NOT_DEFINE_VAR 1
#define NOT_DEFINE_FUNC 2
#define DEFINE_VAR_MULTIPLY 3
#define DEFINE_FUNC_MULTIPLY 4
#define NOT_MATCH_ASSIGNOP 5
#define NOT_LEGAL_LVAL 6
#define NOT_MATCH_OPERATOR 7
#define NOT_MATCH_RETURN 8
#define NOT_MATCH_FUNCPARA 9
#define NOT_ARR_LB 10
#define NOT_FUNC_LC 11
#define NOT_INT_ACCESS_ARR 12
#define NOT_STRUCT_DOT 13
#define NOT_DEFINE_FIELD 14
#define DEFINE_FIELD_MULTIPLY 15
#define DEFINE_STRUCT_MULTIPLY 16
#define NOT_DEFINE_STRUCT 17

void semErrOutput(unsigned int errCode, unsigned int lineNum);
int is_lvalue(Node* node);

#endif 