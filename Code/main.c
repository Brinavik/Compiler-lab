#include "lib.h"
#include "syntax.tab.h"
#include "node.h"
#include "symbol.h"
#include "semantic.h"
#include "trans.h"

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

  /*
    This function is declared in symbol.h,
    and is used to initialize the necessary analysis components. 
    Its definition can be found in symbol.c.
  */
  InitBasicComponents();
  if (has_error == 0){ 
  Program();
#if TRANS_PRINT_DEBUG
    display();
    printf("\n\n\n\n");
#endif

    Trans_Program();
  }
  return 0; 
}
