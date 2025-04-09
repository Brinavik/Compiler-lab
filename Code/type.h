#ifndef TYPE_H
#define TYPE_H

#define TYPE_INT 0
#define TYPE_FLOAT 1

typedef struct Type_* Type;
typedef struct FieldList_* FieldList;

typedef struct Type_ {
    enum {BASIC, ARRAY, STRUCTURE, FUNCTION} kind;
    union {
        int basic; // 0: int, 1: float
        struct {Type elem; int size;} array;
        FieldList fieldlist;
    } u;
    Type retType;   // especially for function return
    char* name;     // especially for structure and function type comparation 
}Type_;

typedef struct FieldList_ {
    char* name;
    Type type;
    FieldList tail;
} FieldList_;

Type create_basic(int basic);
Type create_array(Type elem, int size);
Type create_struct(char* name);
Type create_func(Type retType, char* name);
void append_fieldlist(Type fieldlist, char* name, Type type, unsigned int line);
int cmp_type(Type type1, Type type2);
static int cmp_fieldlist(FieldList list1, FieldList list2);
FieldList find_field_member(Type struct_type, const char* field_name);

#endif