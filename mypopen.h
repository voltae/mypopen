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

/*!
 * @brief simplyfied popen() function, function takes a command and executes the given command in an child process. Commands are liunx/unix shell commands.
 * @param command argument is a pointer to a null-terminated string containing a shell command line. This command is passed to /bin/sh using  the -c  flag;  interpretation, if any, is performed by the shell.
 * @param type a pointer to a null-terminated string  which  must  contain either the letter 'r' for reading or the letter 'w' for writing.
 * @return  FILE Pointer connecting parent and shell in success case, NULL in case it fails
 */
extern FILE *mypopen (const char *command, const char *type);

/*!
 * @brief The mypclose() function waits for the associated process to terminate and returns the exit status of the command as returned by wait(2).
 * @param stream a filestream created by a previous mypopen command,
 * @return the exit status of the terminated child or if the exit status failed, -1
 */
extern int mypclose (FILE *stream);

#endif //MYPOPEN_MPOPEN_H
