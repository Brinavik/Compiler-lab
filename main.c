#include <stdio.h>
#include "syntax.tab.h"
#include "node.h"

extern void yyrestart (FILE*);
extern int yyparse (void);
int has_error = 0;

int main(int argc, char** argv) { 
  if (argc <= 1) return 0; 
  FILE* f = fopen(argv[1], "r"); 
  if (!f) 
  { 
    perror(argv[1]); 
    return 0; 
  } 
  yyrestart(f);
  // yydebug = 1; 
  yyparse(); 
  if (has_error == 0) 
    display();
  return 0; 
}
