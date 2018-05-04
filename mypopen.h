/*!
 * @brief Headerfile of mypopen. All includes are defined here
 * @date 2018-05-04
 * @authors Lara Kammerer  <ic17b001@technikum-wien.at>
 * @authors Valentin Platzgummer <ic17b096@technikum-wien.at>
 */

#ifndef MYPOPEN_MPOPEN_H
#define MYPOPEN_MPOPEN_H

/* ----------------------------------   includes --- */
#include <stdio.h>
#include <errno.h>  // for errno
#include <error.h>
#include <stdio.h>
#include <stdlib.h>  // for exit()
#include <string.h>
#include <unistd.h> // for fork()
#include <sys/types.h> // for getpid() and getppid()
#include <sys/wait.h> // for waitpid
#include <unistd.h>
#include <fcntl.h> // for open
#include <assert.h> // for assert
#include <sys/stat.h>

extern FILE *mypopen (const char *command, const char *type);

extern int mypclose (FILE *stream);



#endif //MYPOPEN_MPOPEN_H
