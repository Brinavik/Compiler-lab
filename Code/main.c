#include "lib.h"
#include "syntax.tab.h"
#include "node.h"
#include "symbol.h"
#include "semantic.h"
#include "trans.h"
#include "trans_mips.h"

extern void yyrestart (FILE*);
extern int yyparse (void);
int has_error = 0;

int main(int argc, char** argv) { 
  if (argc <= 2 || argc >= 4) {
    printf("InputFormat: <parser name> <input> <output>\n");
    return 0;
  } 
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
  /* don't run this. */
  // if(has_error)  assert(0);
 
#if TRANS_PRINT_DEBUG
    display();
    printf("\n\n\n\n");
#endif
 
  // run it for lab3, especially for symbol-table-creation. exit_scope() is also changed.
  Program();

  Trans_Program(argv[2]);

#if TRANS_MIPS_OUTPUT
  Trans_MIPS(argv[2]);
#endif

  return 0; 
}
