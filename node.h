#ifndef _NODE_H_
#define _NODE_H_

typedef enum {SYNTAX_NODE, TOKEN_NODE} NodeType;
typedef struct Node{
    NodeType type; // 0: SyntaxNode, 1: TokenNode
    char* name;
    char* attr; // for SyntaxNode line_num, for TokenNode val or type
    struct Node* child[20];
    int num; // children num
    int depth; // used in display
}Node; 
Node *root;
char buffer[256];


Node* create_node(NodeType type, char* name, char* attr);
Node* create_syntax(NodeType type, char* name,unsigned int line);
void add_node(unsigned int num, Node* parent, ...);
void preorder(Node* node, int depth);
void display();

#endif