# Valentin Platzgummer - ic17b06@technikum-wien.at
# Lara Kammerer - ic17b001technikum-wien.at
# Makefile - mypopen mit Linking zu den test Libraries
# für Verwendung am annuminas server, oder local, wenn die Libraries offline heruntergeladen sind

# Define the required macros
CFLAGS = -Wall -Werror -Wextra -Wstrict-prototypes -Wformat=2 -pedantic -fno-common -ftrapv -O3 -g -std=gnu11
CC = gcc
LDLIBS = -lm
OBJ=mypopen.o
HEADER=mypopen.h
TEST=popentest
TESTPIPE=test-pipe
DOXYGEN=doxygen
CD=cd
MV=mv
RM=rm
GREP=grep
EXCLUDE_PATTERN=footrulewidth


LIB_PATH_TEST=-L./libpopentest
LIB_PATH_PIPE=-L./libtest-pipe
LIB_TEST=-lpopentest
LIB_PIPE=-ltest-pipe


%.c: %o $(HEADER)
	    $(CC) $(CFLAGS) $(HEADER) -c

all: test test-pipe

# link against the testfolder on annuminas
test: $(OBJ)
	    $(CC) -o$(TEST) $(OBJ) $(LIB_TEST) -ldl  $(LIB_PATH_TEST)
        # to run command directly
        #./popentest
# link against the dynamic test suite test-pipe
test-pipe: $(OBJ)
	$(CC) -o$(TESTPIPE) $(OBJ) $(LIB_PIPE) $(LIB_PATH_PIPE)
	    # to run command directly
	    #./test-pipe

.PHONY: clean

clean:
        rm -f *.o mypopen
        rm -f popentest
        rm -f test-pipe

.PHONY: distclean

distclean: clean
        $(RM) -rf doc

# create doxy documentation
doc: html pdf

.PHONY: html

# create html version of documentation
html:
        $(DOXYGEN) doxygen.dcf

# create pdf version of documentation
pdf: html
        $(CD) doc/pdf && \
        $(MV) refman.tex refman_save.tex && \
        $(GREP) -v $(EXCLUDE_PATTERN) refman_save.tex > refman.tex && \
        $(RM) refman_save.tex && \
        make && \
        $(MV) refman.pdf refman.save && \
        $(RM) *.pdf *.html *.tex *.aux *.sty *.log *.eps *.out *.ind *.idx \
          *.ilg *.toc *.tps Makefile && \
        $(MV) refman.save refman.pdf
