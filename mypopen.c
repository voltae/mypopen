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
/// @def max define the buffer for the file reading process
#define BUFFERSIZE 256      // For read and write files
/// @def path to the execution shell
#define EXECPATH "/bin/sh"
/// @def the execution shell itself
#define EXECSHELL "sh"
/// @def execution command
#define EXECCOMM "-c"
// ------------------------------------------------------------------ enum --
/// @enum distinguish between read file and write file
enum operation { M_READ, M_WRITE };

// -------------------------------------------------------------- typedefs --

// --------------------------------------------------------------- globals --

/// @var pid process id number for the current child process, needed because mypclose need to wait for this process
static pid_t pid;

// ------------------------------------------------------------- functions --

static void printError(const char *errorMessage, int lineNumber);
static int commandCheck(const char *command, const char *type);

// check if char is in string
/*static int isCharInString(const char * string);*/

static FILE *ParentPipeStream(int modus, int fd[]);

void ChildPipeStream(int modus, int fd[]);

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
/// @param errorMessage Message to display
/// @return nothing
static void printError(const char *errorMessage, int lineNumber)
{
    if (errorMessage != NULL)
    {
        fprintf(stderr, "%d %s, errno: %d, errdescr: %s\n",lineNumber, errorMessage, errno, strerror(errno));
    }
}
// TODO: delete this code after debugging.
static void testingPrint(const char *testingMessage)
{
    printf("line: %d, message: %s\n", __LINE__, testingMessage);
}

/// @brief simplifiesd implementation of the library command popen
/// @param The  command argument is a pointer to a null-terminated string containing a shell command line.  This command is passed to /bin/sh using  the -c  flag;  interpretation, if any, is performed by the shell.
/// @param type The type argument is a pointer to a null-terminated string  which  must  contain either the letter 'r' for reading or the letter 'w' for writing.
extern FILE *mypopen(const char *command, const char *type)
{
    // create a new buffer for the command
    char commandBuffer[strlen(command)];
    // check if the last element of command is carry return
    if (strncpy(commandBuffer, command, strlen(command)-1) == NULL)
    {
        printError("Error copying th command argument", __LINE__);
    }
    commandBuffer[strlen(commandBuffer)] = 0;

    if (commandCheck(command, type) == 0)
    {
        return NULL;
    }

    // define an array for the file-descriptors of the pipe
    int fd[2];

    // create a new pipe for the program with error checking
    if (pipe(fd) == -1)
    {
        printError("Error in creating pipe", __LINE__);
    }

    testingPrint("file descripors");
    printf("pipe[0]: %d, pipe[1]: %d\n", fd[0], fd[1]);
    // fork the current process. It should always be open only one pipe
    // check if a process is running by asking the pid if is not 0
    if (pid != 0)
    {
        printError("A process is running", __LINE__);
        errno = EAGAIN;
        return NULL;
    }

    // Define the Filepointer for the parent process
    FILE *fp_parent_process = NULL;

    // allocate a buffer for the command sent to the child process
    switch (pid = fork())
    {
        case -1:
            printError("Error in fork process", __LINE__);
            break;

            // we are in the child process
        case 0:
        {
            // if type is "r" the pipe is in read modus, means parent is reading, child is writing
            if (type[0] == 'r')
            {
                // pipe is in write mode from childs perspective
                ChildPipeStream(M_WRITE, fd);
            }
                // pipe is in read mode from childs perspective
            else if (type[0] == 'w')
            {
                ChildPipeStream(M_READ, fd);
            }
                // default path type is missuses, should not happen
            else
            {
                assert(0);
            }
            // execute the current command.
            execl(EXECPATH, EXECSHELL, EXECCOMM, commandBuffer, NULL);

            // if we get to this point, ann error occurred,
            printError("Error in execute line", __LINE__);
            break;
        }

            // we are in the parent process
        default:
        {
            // if type is "r" the pipe is in read modus from parents perspective
            if (type[0] == 'r')
            {
                fp_parent_process = ParentPipeStream(M_READ, fd);
            }
            else if (type[0] == 'w')
            {
                fp_parent_process = ParentPipeStream(M_WRITE, fd);
            }
                // default path, should not happen
            else
            {
                assert(0);
            }
            break;
        }
    }

    return fp_parent_process;
}

/// @brief The pclose() function waits for the associated process to terminate and returns the exit status of the command as returned by wait4(2).
/// @returns the exit staus of the wait system call
extern int mypclose(FILE *stream)
{

    // a process with pipe is already running
    if (pid != 0)
    {
        errno = ECHILD;
        exit(EXIT_FAILURE);
    }
    int status = 0;
    if ((waitpid(pid, &status, WUNTRACED)) == -1)
    {
        printError("Error in waitpid", __LINE__);
        // set errno to invalid child, if the child pid is not the one we are waiting for.
        errno = EAGAIN;
    }
    // reset pid to zero so it can be tested if a fork is already running
    pid = 0;

    // close file Pointer
    if (fclose(stream) == EOF)
    {
        printError("Error in close file", __LINE__);
    }

   exit(status);
}


/// @brief configure the parent process
///
/// @param modus ste the parent either to read 0 or write 1
/// @param fd[] array of file descriptors
///
/// @return file Pointer to stream on sucess, null on failure
/// @error errno is set appropately
static FILE *ParentPipeStream(int modus, int fd[])
{
    errno = 0;
    FILE *parentStream = NULL;
    switch (modus)
    {
        // parent process in read modus
        case M_READ:
        {
            // close the write end of the pipe
            if (close(fd[M_WRITE] == -1))
            {
                printError("Error in close write Descriptor", __LINE__);

            }
            // try to open a file stream to read the pipe
            if ((parentStream = fdopen(fd[M_READ], "r")) == (FILE *) NULL)
            {
                printError("Error in read pipe", __LINE__);
            }
            break;
        }
            // parent process in write modus
        case M_WRITE:
        {

            if (close(fd[M_READ] == -1))
            {
                printError("Error in close read Descriptor", __LINE__);
                printf("read: %d, write: %d\n",fd[M_READ], fd[M_WRITE]);
            }
            // try to open a file stream to read the pipe
            if ((parentStream = fdopen(fd[M_WRITE], "w")) == (FILE *) NULL)
            {
                printError("Error in write pipe", __LINE__);
            }
            break;
        }

        // error catch, this part should never be executed
        default:
        {
            assert(0);
            break;
        }
    }

    return parentStream;
}

/// @brief Create the file stream for the child process
/// @param modus 0 for read, 1 for write
/// @return nothing
/// @error errno is set, function leaves with exit failure
void ChildPipeStream(int modus, int fd[])
{
    errno = 0;
    switch (modus)
    {
        // child process in write modus
        case M_READ:
        {
            if (close(fd[M_READ]) == -1)
            {
                printError("Error in close write Descriptor", __LINE__);
            }

            // Redirect the stdout to the pipe in order to read from pipe and check error
            if (dup2(fd[M_WRITE], STDOUT_FILENO) == -1)
            {
                printError("Error in duplicating stout", __LINE__);
            }
            break;

        }
            // child process in read modus
        case M_WRITE:
        {
            if (close(fd[M_WRITE]) == -1)
            {
                printError("Error in close read Descriptor", __LINE__);
            }

            // redirect the stdin to the the read input of the pipe in order to write to the pipe
            if (dup2(fd[M_READ], STDIN_FILENO) == -1)
            {
                printError("Error in duplicating stdin", __LINE__);
            }
            break;
        }
            // error catch, this part should never be executed
        default:
        {
            assert(0);
            break;
        }
    }
}

static int commandCheck(const char *command, const char *type)
{
    //first do the correct type check. type is only 1 element long and either 'w' or 'r'
    if ((type[0] != 'w' && type[0] != 'r') || type[1] != 0)
    {
        printError("Type check wrong", __LINE__);
        // set errno to invalid operation
        errno = EINVAL;
        return 0;
    }
        // or the type is longer than 1 character
    else if (strlen(type) > 1)
    {
        printError("Type check wrong", __LINE__);
        // set errno to invalid operation
        errno = EINVAL;
        return 0;
    }

    if (command == NULL)
    {
        errno = EINVAL;
        return 0;
    }

    return 1;


}