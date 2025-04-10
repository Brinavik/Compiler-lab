#include <stdlib.h>
#include <string.h>
#include "type.h"

Type create_basic(int basic) {
    Type type = (Type)calloc(1, sizeof(Type_));
    type->kind = BASIC;
    type->u.basic = basic;

    return type;
}

Type create_array(Type elem, int size) {
    Type type = (Type)calloc(1, sizeof(Type_));
    type->kind = ARRAY;
    type->u.array.elem = elem;
    type->u.array.size = size;
    return type;
}

Type create_struct(char* name) {
    Type type = (Type)malloc(sizeof(Type_));
    type->kind = STRUCTURE;
    type->u.fieldlist = NULL;
    type->name = strdup(name);
    return type;
}

// the first field is return value 
Type create_func(Type retType, char* name) {
    Type type = (Type)calloc(1, sizeof(Type_));
    type->kind = FUNCTION;
    type->u.fieldlist = NULL;
    type->retType = retType;
    type->name = strdup(name);
    return type;
}

FieldList find_field_member(Type struct_type, char* name) {
    FieldList cur = struct_type->u.fieldlist;
    while (cur != NULL) {
        if (strcmp(name, cur->name) == 0) {
            return cur;
	}
	cur = cur->tail;
    }
    return NULL;
}

void append_fieldlist(Type fieldlist, char* name, Type type) {
    FieldList new_field = (FieldList)calloc(1, sizeof(FieldList_));
    new_field->name = strdup(name);
    new_field->type = type;
    new_field->tail = fieldlist->u.fieldlist;
    fieldlist->u.fieldlist = new_field;
}


int cmp_type(Type type1, Type type2) {
    // both function
    if (type1->kind == FUNCTION && type2->kind == FUNCTION) {
        return cmp_type(type1->retType, type2->retType);
    }
    // either function
    else if (type1->kind == FUNCTION) {
        return cmp_type(type1->retType, type2);
    }
    else if (type2->kind == FUNCTION) {
        return cmp_type(type1, type2->retType);
    }
    // neither function
    else {
        if (type1->kind != type2->kind)
            return 0;
        switch (type1->kind) {
            case BASIC:
                return type1->u.basic == type2->u.basic;
            case ARRAY:
                return cmp_type(type1->u.array.elem, type2->u.array.elem) && 
                       type1->u.array.size == type2->u.array.size;
            case STRUCTURE:
                if (strcmp(type1->name, type2->name) != 0)
                    return 0;
                return cmp_fieldlist(type1->u.fieldlist, type2->u.fieldlist);
            default:
                return 0;
        }
    }
    return 0;
}


static int cmp_fieldlist(FieldList list1, FieldList list2) {
    while (list1 != NULL && list2 != NULL) {
        if (strcmp(list1->name, list2->name) != 0 || 
            !cmp_type(list1->type, list2->type))
            return 0;
        list1 = list1->tail;
        list2 = list2->tail;
    }
    return list1 == list2; 
}