# Define the required macros
CFLAGS = -Wall -Werror -Wextra -Wstrict-prototypes -Wformat=2 -pedantic -fno-common -ftrapv -O3 -g -std=gnu11
CC = gcc
LDLIBS = -lm
OBJ=mypopen.o
HEADER=mypopen.h
TEST=popentest
TESTPIPE=test-pipe

LIB_PATH_TEST=-L./libpopentest
LIB_PATH_PIPE=-L./libtest-pipe
LIB_TEST=-lpopentest
LIB_PIPE=-ltest-pipe


%.c: %o
	    $(CC) $(CFLAGS) -c

all: test test-pipe

# link against the testfolder on annuminas
test: $(OBJ)
	    $(CC) -o$(TEST) $(OBJ) $(LIB_TEST) -ldl  $(LIB_PATH_TEST)
        # to run command directly
        #./popentest
# link against the dynamic test suite test-pipe
test-pipe: $(OBJ)
	$(CC) -o$(TESTPIPE) $(OBJ) $(LIB_PIPE)    $(LIB_PATH_PIPE)
	    # to run command directly
	    #./test-pipe

.phony : clean

clean:
	rm -f *.o mypopen


# todo: insert doxy generator rule
