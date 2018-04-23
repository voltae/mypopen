//*
// @file mypopen
// Betriebssysteme mypopen/mypclose File.
// Beispiel 0
//
// @author Valentin Platzgummer <ic17b096@technikum-wien.at>
// @author Lara Kammerer <ic17b001@technikum-wien.at>
// @date 2018/04/23
//
// @version 1
//
// @todo Test it more seriously and more complete.
// @todo Review it for missing error checks.
// @todo Review it and check the source against the rules at
//       https://cis.technikum-wien.at/documents/bic/2/bes/semesterplan/lu/c-rules.html
//

// -------------------------------------------------------------- includes --

#include <errno.h>
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
#include "mypopen.h"


// --------------------------------------------------------------- defines --

/// @def distinguish between read file and write file
/// 0 for read and 1 for write
#define M_READ 0
#define M_WRITE 1

/// @def max define the buffer for the file reading process
#define BUFFERSIZE 256      // For read and write files
/// @def path to the execution shell
#define EXECPATH "/bin/sh"
/// @def the execution shell itself
#define EXECSHELL "sh"
/// @def execution command
#define EXECCOMM "-c"
// ------------------------------------------------------------------ enum --


// -------------------------------------------------------------- typedefs --

// --------------------------------------------------------------- globals --

/// @var pid process id number for the current child process, needed because mypclose need to wait for this process
static pid_t pid;

// ------------------------------------------------------------- functions --

static void printError (const char *errorMessage);
static FILE * ParentPipeStream (int modus, int *fd);
static FILE * ChildPipeStream (int modus, int *fd);

//*
// \brief The most minimalistic C program
//
// This is the main entry point for any C program.
//
// \param argc the number of arguments
// \param argv the arguments itselves (including the program name in argv[0])
//
// \return always "success"
// \retval 0 always
//


/// @brief function prints out an error message
/// @param errorMessage Message to diplay
/// @return nothing leaves the program wit EXIT exit failure
static void printError (const char *errorMessage)
{
    if (errorMessage != NULL)
    {
        printError(errorMessage);
    }

    exit(EXIT_FAILURE);
}


// todo: if usage is not correct, do we print out usage message?
static void usageError (void)
{
    printError("Usage: mypopen (command, type)");
}


/// @brief simplified implementation of the library command popen
/// @param The  command argument is a pointer to a null-terminated string containing a shell command line.  This command is passed to /bin/sh using  the -c  flag;  interpretation, if any, is performed by the shell.
/// @param type The type argument is a pointer to a null-terminated string  which  must  contain either the letter 'r' for reading or the letter 'w' for writing.
extern FILE *mypopen (const char *command, const char *type)
{
    //first do the correct type check. type is only 1 element long and either 'w' or 'r'
    if (strlen (type) > 1  || (strncmp(type, "w", 1) == 0) || (strncmp(type, "r", 1)) == 0)
    {
        printError("Wrong use of type");
    }

    // change the type to the define created above


    // define an array for the file-descriptors of the pipe
    int fd[2];

    // create a new pipe for the program with error checking
    if (pipe(fd) == -1)
    {
        printError("Error during create pipe");
    }

    // fork the current process. It should always be open only one pipe
    // check if a process is running by asking the pid if is not 0
    if (pid != 0)
    {
        printError("A process is running");
    }

    // Define the Filepointer for the to processes
    FILE *fp_parent_process = NULL;
    FILE *fp_child_process = NULL;

    // allocate a buffer for the command sent to the child process
   // char commandBuffer[BUFFERSIZE];
    switch (pid = fork())
    {
        case -1:
            printError("Error in fork process");
            break;

            // we are in the child process
        case 0:
        {
            // if type is "r" the pipe is in read modus, means parent is reading, child is writing
            if (strncmp(type, "r", 1) == 0)
            {
                // pipe is in write mode from childs perspective
                fp_child_process = ChildPipeStream(M_WRITE, fd);
            }
                // pipe is in read mode from childs perspective
            else if (strncmp(type, "w", 1) == 0)
            {
                fp_child_process = ChildPipeStream(M_READ, fd);
            }
            // default path type is missuses, should not happen
            else
            {
                assert(0);
            }

            // execute the current command.
            execl(EXECPATH, EXECSHELL, EXECCOMM, command);
            // if we get to this point, ann error occurred,
            printError("Error in execute line");
        }

        // we are in the parent process
        default:
        {
            // if type is "r" the pipe is in read modus from parents perspective
            if (strncmp(type, "r", 1) == 0)
            {
                fp_parent_process = ParentPipeStream(M_READ, fd);
            }
            else if (strncmp(type, "w", 1) == 0)
            {
                fp_parent_process = ParentPipeStream(M_WRITE, fd);
            }
                // default path, should not happen
            else
            {
               assert(0);
            }
        }
    }

    return fp_parent_process;
}

extern int mypclose (FILE *stream)
{
    int status;
    if ((waitpid(pid, &status, WUNTRACED)) == -1)
    {
        printError("Error in waitpid");
    }

    // reset pid to zero so it can be tested if a fork is already running
    pid = 0;


    // close file Pointer
    if (fclose(stream) == EOF)
    {
        printError("Error in close file");
    }

    exit(EXIT_SUCCESS);
}


/// @brief configure the parent process
///
/// @param modus ste the parent either to read 0 or write 1
/// @param fd[] array of file descriptors
///
/// @return file Pointer to stream on sucess, null on failure
/// @error errno is set, function leaves with exit failure
static FILE * ParentPipeStream (int modus, int *fd)
{
    errno = 0;
    FILE *parentStream = NULL;
    switch (modus)
    {
        // parent process in read modus
        case M_READ:
        {
            // close the rwrite end of the pipe
            if (close(fd[M_WRITE] == -1))
            {
                printError("Error in close write Descriptor");

            }
            // try to open a file stream to read the pipe
            if ((parentStream = fdopen(fd[M_READ], "r")) == (FILE *)NULL)
            {
                printError("Error in read pipe");
            }
        }
            // parent process in write modus
        case M_WRITE:
        {

            if (close(fd[M_READ] == -1))
            {
                printError("Error in close read Descriptor");

            }
            // try to open a file stream to read the pipe
            if ((parentStream = fdopen(fd[M_WRITE], "w")) == (FILE *)NULL)
            {
                printError("Error in write pipe");
            }
        }

    }

    return parentStream;
}

/// @brief Create the file stream for the child process
/// @param modus 0 for read, 1 for write
/// @return filestream child process
/// @error errno is set, function leaves with exit failure
static FILE * ChildPipeStream (int modus, int *fd)
{
    errno = 0;
    FILE *childStream = NULL;
    switch (modus)
    {
        // child process in read modus
        case M_READ:
        {
            if (close(fd[M_WRITE]) == -1)
            {
                printError("Error in close write Descriptor");

            }
            // try to open a file stream to read the pipe
            if ((childStream = fdopen(fd[M_READ], "r")) == (FILE *)NULL)
            {
                printError("Error in read pipe");
            }

            // Redirect the stdin to the pipe in order to read from pipe and check error
            if (dup2(fd[M_READ], STDIN_FILENO) == -1)
            {
                printError("Error in duplicating stdin");
            }

        }
            // parent process in write modus
        case M_WRITE:
        {
            if (close(fd[M_READ]) == -1)
            {
                printError("Error in close read Descriptor");
            }
            // try to open a file stream to read the pipe
            if ((childStream = fdopen(fd[M_WRITE], "w")) == (FILE *)NULL)
            {
                printError("Error in write pipe");
            }

            // redirect the stdout to the the read input of the pipe in order to write to the pipe
            if (dup2(fd[M_WRITE], STDOUT_FILENO) == -1)
            {
                printError("Error in duplicating stdout");
            }
        }
    }

    return childStream;
}
