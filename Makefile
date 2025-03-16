CC = gcc
LEX = flex
YACC = bison -d 
CFLAGS = 
LDFLAGS = -lfl -ly
TARGET = ../parser
SOURCES = main.c syntax.tab.c tree.c
OBJ = $(SOURCES:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $^ $(LDFLAGS) -o $@

syntax.tab.c syntax.tab.h: syntax.y
	$(YACC) $<

lex.yy.c: lexical.l
	$(LEX) $<

# 显式声明依赖关系
main.o: main.c syntax.tab.h tree.h lex.yy.c
syntax.tab.o: syntax.tab.c syntax.tab.h
tree.o: tree.c tree.h

clean:
	rm -f $(TARGET) $(OBJ) lex.yy.c syntax.tab.c syntax.tab.h

.PHONY: all clean