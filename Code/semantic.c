#include <stdlib.h>
#include "node.h"
#include "type.h"
#include "semantic_analysis_error.h"
#include "symbol.h"

extern Node* root;

void Program() {
  ExtDefList(root->child[0]);
}

void ExtDefList(Node* node) {
    if (node == NULL) return;
    ExtDef(node->child[0]);
    ExtDefList(node->child[1]);
}

void ExtDef(Node* node) {
    Type type = Specifier(node->child[0]);
    if (strcmp(node->child[1]->name, "ExtDecList") == 0) {
        ExtDecList(node->child[1], type);
    } else if (strcmp(node->child[1]->name, "FunDec") == 0) {
        FunDec(node->child[1], type);
	Compst(node->child[2]);
    }
}

Type FunDec(Node* node, Type retType) {
    Type func = create_func(retType, node->child[0]->attr);
    if((node->child[2]->name, "VarList") == 0) {
        VarList(node->child[2], func);
    }
    insert_symbol(node->child[0]->attr, node->lineNum, func);
    return func;
}

void VarList(Node* node, Type func) {
    ParamDec(node->child[0], func);
    if (node->num == 3) {
        VarList(node->child[2], func);
    }
}

void ParamDec(Node* node, Type func) {
    Type type = Specifier(node->child[0]);
    VarDec(node->child[1], type, func);
}

void CompSt(Node* node) {
    DefList(node->child[1], NULL);
    StmtList(node->child[2]);
}

void StmtList(Node* node) {
    if (node != NULL) {
        Stmt(node->child[0]);
	StmtList(node->child[1]);
    }
}

void Stmt(Node* node) {
    if (node->num == 2) {
        Exp(node->child[0]);
    } else if (node->num == 1) {
        CompSt(node->child[0]);
    } else if (node->num == 3) {
        // TODO
	Exp(node->child[2]);
    } else if (node->num == 5) {
        Exp(node->child[2]);
	Stmt(node->child[4]);
    } else if (strcmp(node->child[6]->name, "Stmt")) {
        Exp(node->child[2]);
	Stmt(node->child[4]);
	Stmt(node->child[6]);
    }
}


void ExtDecList(Node* node, Type type) {
    VarDec(node->child[0], type, NULL);
    if (node->num == 3) {
        ExtDecList(node->child[2], type);
    }
}

Type Specifier(Node* node) {
    if (strcmp(node->child[0]->name, "TYPE")) {
        if (strcmp(node->child[0]->attr, "int")) {
            return create_basic(TYPE_INT);
	}
	if (strcmp(node->child[0]->attr, "float")) {
            return create_basic(TYPE_FLOAT);
	}
    } else if (strcmp(node->child[0]->name, "StructSpecifier")) {
        return StructSpecifier(node->child[0]);
    }
}

Type StructSpecifier(Node* node) {
    if (node->num == 5) {
        Type structure = OptTag(node->child[1]);
        if (structure != NULL) {
            DefList(node->child[3], structure);
	    }
        else {
            printf("Illegal OptTag?\n");
            assert(0);
	    }
    } 
    else if (node->num == 2) {
        return Tag(node->child[1]);
    }
}

Type OptTag(Node* node) {
    if (node != NULL) {
        Type ret = create_struct(node->child[0]->attr);
	    insert_symbol(node->child[0]->attr, node->lineNum, ret);
        return ret;
    }
    return NULL;
}

Type Tag(Node* node) {
    char* struct_name = node->child[0]->attr;
    if(node->child[0]->type != STRUCTURE)
        assert(0);
    SymbolEntry* entry = lookup_symbol_with_a_type(struct_name, STRUCTURE);
    
    if (entry == NULL) {
        semErrOutput(NOT_DEFINE_STRUCT, node->lineNum);
        return NULL;
    }
    return entry->type;
}

void DefList(Node* node, Type structure) {
    if (node != NULL) {
        Def(node->child[0], structure);
	DefList(node->child[1], structure);
    }
}

void Def(Node* node, Type structure) {
    Type type = Specifier(node->child[0]);
    DecList(node->child[1], type, structure);
}

void DecList(Node* node, Type type, Type structure) {
    if (node->num == 1) {
        Dec(node->child[0], type, structure);
    } else {
        Dec(node->child[0], type, structure);
	DecList(node->child[2], type, structure);
    }
}

void Dec(Node* node, Type type, Type structure) {
    if (node->num == 1) {
        VarDec(node->child[0], type, structure);    
    } else {
        // TODO
    }
}

void Args(Node* node, Type type) {

    // Args: NEED TO BE COMPLETED  

    if(node->num == 1)
        Exp(node->child[0]);
    else if(node->num == 3) {
        Exp(node->child[0]);
        Args(node->child[2], type);
    }
    else 
        assert(0);
}


void VarDec(Node* node, Type type, Type fieldlist) {
    if (node->num == 1) {
        if (fieldlist != NULL) {
	    append_fieldlist(fieldlist, node->child[0]->attr, type, node->child[0]->lineNum);
	} else {
            insert_symbol(node->child[0]->attr, node->lineNum, type);
	}
    } else {
        Type array = create_array(type, node->child[2]->attr);
        if (fieldlist != NULL) {
	    append_fieldlist(fieldlist, atoi(node->child[0]->attr), array, node->child[0]->lineNum);
	} else {
            insert_symbol(node->child[0]->attr, node->lineNum, array);
	}
    }
}




Type Exp(Node* node) {
    if (node == NULL) return NULL;

    if (node->num == 1) {
        // get an ID
        if (strcmp(node->child[0]->name, "ID") == 0) {
            SymbolEntry* symbol = lookup_symbol(node->child[0]->attr);
            if(!symbol) {
                semErrOutput(NOT_DEFINE_VAR, node->lineNum);   
                return NULL;
            }
            return symbol->type;
        } else if (strcmp(node->child[0]->name, "INT") == 0) {
            return create_basic(TYPE_INT);
        } else if (strcmp(node->child[0]->name, "FLOAT") == 0) {
            return create_basic(TYPE_FLOAT);
        }
    }
    // LP Exp RP
    if (node->num == 3 && strcmp(node->child[0]->name, "LP") == 0) {
        return Exp(node->child[1]); 
    }

    if (node->num == 2) {
        Type type = Exp(node->child[1]);
        if (strcmp(node->child[0]->name, "MINUS") == 0) {
            if (type->kind != BASIC) {
                semErrOutput(NOT_MATCH_OPERATOR, node->lineNum);
                return NULL;
            }
            return type;
        }
        if (strcmp(node->child[0]->name, "NOT") == 0) {
            if (type->kind != BASIC || type->u.basic != TYPE_INT) {
                semErrOutput(NOT_MATCH_OPERATOR, node->lineNum);
                return NULL;
            }
            return create_basic(TYPE_INT);
        }
    }

    if (node->num == 3 && is_operator(node->child[1])) {
        Type left = Exp(node->child[0]);
        Type right = Exp(node->child[2]);

        // Assignop
        if (strcmp(node->child[1]->name, "ASSIGNOP") == 0) {
            
            // left-value check
            if (!is_lvalue(node->child[0])) {
                semErrOutput(NOT_LEGAL_LVAL, node->lineNum);
                return NULL;
            }

            // Match types on left and right sides
            if(!cmp_type(left, right)) {
                semErrOutput(NOT_MATCH_ASSIGNOP, node->lineNum);
                return NULL;
            }
            return right;
        }
        
        if (node->child[1]->name == "PLUS" || node->child[1]->name == "MINUS" ||
            node->child[1]->name == "STAR" || node->child[1]->name == "DIV") {    
            
                return left;
        } 
        else if (node->child[1]->name == "AND" || node->child[1]->name == "OR") {
            return create_basic(TYPE_INT);
        } 
        else if (node->child[1]->name == "RELOP") {
            return create_basic(TYPE_INT);
        }
    }

    // about array access
    if (node->num == 4 && strcmp(node->child[1]->name, "LB") == 0) {
        Type array_type = Exp(node->child[0]);
        Type index_type = Exp(node->child[2]);
        
        if (array_type->kind != ARRAY) {
            semErrOutput(NOT_ARR_LB, node->lineNum);
            return NULL;
        }
        if(index_type->kind != BASIC || index_type->u.basic != TYPE_INT) {
            semErrOutput(NOT_INT_ACCESS_ARR, node->lineNum);
            return NULL;
        }
        return array_type->u.array.elem;
    }

    // structure member access
    if (node->num == 3 && strcmp(node->child[1]->name, "DOT") == 0) {
        Type struct_type = Exp(node->child[0]);
        if (struct_type->kind != STRUCTURE) {
            semErrOutput(NOT_STRUCT_DOT, node->lineNum);
            return NULL;
        }
        
    // continue to complete this: find_member()
        FieldList field = find_field_member(struct_type, node->child[2]->attr);
        if (!field){
            semErrOutput(NOT_DEFINE_FIELD, node->lineNum);
            return NULL;
        } 
        return field->type;
    }

    // about function
    if (node->num >= 3 && strcmp(node->child[1]->name, "LP") == 0) {
        SymbolEntry* func = lookup_symbol_with_a_type(node->child[0]->attr, FUNCTION);
        if (!func) {
            func = lookup_symbol(node->child[0]->attr);
            if (func != NULL && func->type->kind != FUNCTION) {
                semErrOutput(NOT_FUNC_LC, node->lineNum);
                return NULL;
            }
            semErrOutput(NOT_DEFINE_FUNC, node->lineNum);
            return NULL;
        }
        
        if (node->num == 4) { 
            if (func->type->u.fieldlist != NULL){
                semErrOutput(NOT_MATCH_FUNCPARA, node->lineNum);
                return NULL;
            }  
        } else {
            check_arguments(func->type->u.fieldlist, Args(node->child[2]));
        }
        return func->type;
    }


    printf("Unknown error case of Exp!\n");
    assert(0);
    return NULL;
}