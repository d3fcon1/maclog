SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)
DEP = $(OBJ:.o=.d)
CC = clang
CFLAGS = -g -Wall -MMD -I../include
LDFLAGS = -dynamiclib
FRAMEWORKS = -framework CoreGraphics -framework CoreFoundation -framework Carbon

../lib/libmaclog.dylib: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(FRAMEWORKS)
	dsymutil ../lib/libmaclog.dylib

-include $(DEP)

.PHONY: clean
clean:
	rm -rf $(OBJ) $(DEP) ../lib/libmaclog.dylib ../lib/libmaclog.dSYM

