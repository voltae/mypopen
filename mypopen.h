//
// Created by marcaurel on 21.04.18.
//

#include <stdio.h>

#ifndef MYPOPEN_MPOPEN_H
#define MYPOPEN_MPOPEN_H

extern FILE *mypopen (const char *command, const char *type);

extern int mypclose (FILE *stream);



#endif //MYPOPEN_MPOPEN_H
