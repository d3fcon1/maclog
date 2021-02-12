SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)
DEP = $(OBJ:.o=.d)
CC = clang
CFLAGS = -g -Wall -MMD -I../include
LDFLAGS = -L../lib
LDLIBS = -lmaclog

../bin/maclog-test: maclog-test.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)
	dsymutil ../bin/maclog-test
	
-include $(DEP)

.PHONY: clean
clean:
	rm -rf $(OBJ) $(DEP) 
	rm -rf ../bin/maclog-test ../bin/maclog-test.dSYM

