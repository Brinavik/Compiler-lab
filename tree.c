#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include "tree.h"

Node* root;
char buffer[256];

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
