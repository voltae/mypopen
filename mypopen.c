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
#include <sys/stat.h>
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
enum operation
{
    M_READ, M_WRITE
};

// -------------------------------------------------------------- typedefs --

/// @enum distinuish between valid and invalid arguments
typedef enum isValid
{
    INVALID = -1, VALID
} isValid;

/*!
 * @brief structure to hold the pid and the file descriptor of the actual child process
 */
typedef struct childProcess
{
    pid_t parentpid;
    pid_t childpid;
    int *fd;
    FILE *filepointer;
} childProcess;

// --------------------------------------------------------------- globals --

/// @var pid process id number for the current child process, needed because mypclose need to wait for this process
//static pid_t pid = -1;
/*!
 * @brief pointer to the structure to check if child exists or not
 */
static childProcess *actualProcess;
// ------------------------------------------------------------- functions --

static void printError(const char *errorMessage, int lineNumber);

static isValid commandCheck(const char *command, const char *type);

// check if char is in string
/*static int isCharInString(const char * string);*/

static FILE *ParentPipeStream(int modus, int fd[]);

void ChildPipeStream(int modus, int fd[]);



/*!
* @brief Implementation of a simplifed popen() library function
*
* This is the main entry point for any C program.
*
* @param command the number of arguments
* @param type the arguments itselves (including the program name in argv[0])
*
* @retval FILE Pointer connecting parent and shell in success case
* @retval NULL in case it fails
*
*/
/*!
 * @brief Function prints out Messages for debugging including line number
 * @param errorMessage char pointer with the message text
 * @param lineNumber integer with the lin number the message comes from
 */
static void printError(const char *errorMessage, int lineNumber)
{
    if (errorMessage != NULL)
    {
        fprintf(stderr, "%d %s, errno: %d, errdescr: %s\n", lineNumber, errorMessage, errno, strerror(errno));
    }
}
// TODO: delete this code after debugging.
/*static void testingPrint(const char *testingMessage)
{
    printf("line: %d, message: %s\n", __LINE__, testingMessage);
}*/

/// @brief simplifiesd implementation of the library command popen
/// @param The  command argument is a pointer to a null-terminated string containing a shell command line.  This command is passed to /bin/sh using  the -c  flag;  interpretation, if any, is performed by the shell.
/// @param type The type argument is a pointer to a null-terminated string  which  must  contain either the letter 'r' for reading or the letter 'w' for writing.
extern FILE *mypopen(const char *command, const char *type)
{
    // set errno to 0 to capture the right error message
    errno = 0;

    // if no actual process is allocated, allocate a new childprocess struct
    if (actualProcess == NULL)
    {
        if ((actualProcess = malloc(sizeof(childProcess))) == NULL)
        {
            errno = ECHILD;
            return NULL;
        }
    }

    // check if a process is running by asking the pid if is not 0
    if (actualProcess->filepointer != NULL)
    {
        errno = EAGAIN;
        return NULL;
    }
    // Check if the given arguments are valid
    if (commandCheck(command, type) == INVALID)
    {
        errno = EINVAL;
        fprintf(stderr, "Illegal input: %s", strerror(errno));
        return NULL;
    }

    // define an array for the file-descriptors of the pipe
    int fd[2];

    // create a new pipe for the program with error checking
    if (pipe(fd) == -1)
    {
        printError("Error in creating pipe", __LINE__);
        errno = ECHILD;
        return NULL;
    }

    // Define the Filepointer for the parent process
    FILE *fp_parent_process = NULL;

    // fork the current process. It should always be open only one pipe
    pid_t pid = actualProcess->childpid = fork();
    // store the parent pid of the current process,
    actualProcess->parentpid = getppid();

    switch (pid)
    {
        case -1:
            printError("Error in fork process", __LINE__);
            // Closing the pipes end if the fork process failed
            close(fd[M_READ]);
            close(fd[M_WRITE]);
            return NULL; //break what for??----------------------------------------------------------------------------------------------------------------------
            break;

            // we are in the child process
        case 0:
        {
            // if type is "r" the pipe is in read modus, means parent is reading, child is writing
            if (type[0] == 'r')
            {
                ChildPipeStream(M_READ, fd);
            }
                // pipe is in read mode from childs perspective
            else if (type[0] == 'w')
            {
                ChildPipeStream(M_WRITE, fd);
            }
                // default path type is missuses, should not happen
            else
            {
                assert(0);
            }

            // execute the current command.
            execl(EXECPATH, EXECSHELL, EXECCOMM, command, (char *) NULL);

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
    // Commenting out waiting for pid in parent process for testing reasons
    /*// capture the wait state only for debugging
    int status;
    pid_t childPid = waitpid(actualProcess->childpid, &status, WNOHANG);
    if (childPid == -1)
    {
        printError("Exit failed", __LINE__);
        return NULL;
    }

    if (WIFEXITED(status))
    {
        // return file stream only if the child terminated properly
        if (WEXITSTATUS(status) == EXIT_SUCCESS)
        {
            // Storing the filepointer in the actual child struct

            actualProcess->filepointer = fp_parent_process;
            fprintf(stdout, "file pointer: %p\n", (void *)fp_parent_process);
            return fp_parent_process;
        }
        else if (WEXITSTATUS(status) == EXIT_FAILURE)
        {
            printf("Terminated with error: %d, %s", errno, strerror(errno));
            //exit(EXIT_FAILURE);
        }
        else if (WIFSTOPPED(status))
        {
            printf("stopped by signal %d\n", WSTOPSIG(status));
        }
        else if (WIFCONTINUED(status))
        {
            printf("continued\n");
        }

    }*/
    // Storing the filepointer in the actual child struct
    actualProcess->filepointer = fp_parent_process;
    return fp_parent_process;
    //return NULL;
}

/// @brief The pclose() function waits for the associated process to terminate and returns the exit status of the command as returned by wait4(2).
/// @returns the exit status of the wait system call
extern int mypclose(FILE *stream)
{
    // If mypclose is called without any actual process open
    if (stream == NULL && actualProcess == NULL)
    {
        errno = EINVAL;
        return INVALID;
    }
        // There was called mypopen before, but no file pointer generated
    else if (stream == NULL)
    {
        errno = ECHILD;
        return INVALID;
    }
    // there is no child at all
    if (actualProcess == NULL)
    {
        errno = ECHILD;
        return INVALID;
    }
    // Check if the incoming stream is created with mypopen
    if (stream != actualProcess->filepointer)
    {
        errno = EINVAL;
        return INVALID;
    }

    close(actualProcess->fd[M_WRITE]);

    int status = 0;
    pid_t waitedChild;

    /* wait for terminating properly the child process */
    do
    {
        waitedChild = waitpid(actualProcess->childpid, &status, 0);

    } while (waitedChild == -1 && errno == EINTR);

    printf("Exit status: %d\n", status);
    if (WIFEXITED(status))
    {
        // deallocate the struct
        free(actualProcess);
        actualProcess = NULL;

        // close file Pointer
        if (fclose(stream) == EOF)
        {
            printError("Error in close file", __LINE__);
            return INVALID;
        }

        stream = NULL;

        return WEXITSTATUS(status);
    }
    // deallocate the struct
    free(actualProcess);
    actualProcess = NULL;

    // close file Pointer
    if (fclose(stream) == EOF)
    {
        printError("Error in close file", __LINE__);
        return INVALID;
    }

    stream = NULL;
    return INVALID;
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
            int statusCloseWrite = close(fd[M_WRITE]);
            if (statusCloseWrite == -1)
            {
                printError("Error in close write Descriptor", __LINE__);

            }
            // try to open a file stream to read the pipe
            actualProcess->fd = fd;
            parentStream = fdopen(fd[M_READ], "r");
            if (parentStream == NULL)
            {
                // close the read end of the parents pipe
                close(fd[M_READ]);
                printError("Error in read pipe", __LINE__);
                return NULL;
            }
            break;
        }
            // parent process in write modus
        case M_WRITE:
        {
            int statusCloseRead = close(fd[M_READ]);
            if (statusCloseRead == -1)
            {
                printError("Error in close read Descriptor", __LINE__);
                printf("read: %d, write: %d\n", fd[M_READ], fd[M_WRITE]);
            }
            // try to open a file stream to read the
            actualProcess->fd = fd;
            parentStream = fdopen(fd[M_WRITE], "w");
            if (parentStream == NULL)
            {
                // close writes end of the parents pipe
                close(fd[M_WRITE]);
                printError("Error in write pipe", __LINE__);
                return NULL;
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

    // store the file descriptor for the current process
    actualProcess->fd = fd;

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
            // pipe is in write mode from childs perspective
            int statusCloseRead = close(fd[M_READ]);
            if (statusCloseRead == -1)
            {
                printError("Error in close read Descriptor", __LINE__);
            }

            // Redirect the stdout to the pipe in order to read from pipe and check error
            if (fd[M_WRITE] != STDOUT_FILENO)
            {
                int statusDupWrite = dup2(fd[M_WRITE], STDOUT_FILENO);
                if (statusDupWrite == -1)
                {
                    // close the childs write pipe end in chase the dup failed.
                    close(M_WRITE);
                    printError("Error in duplicating stout", __LINE__);
                }
            }
            // After redirection close the filedescriptor fd write
            int statusCloseWrite = close(fd[M_WRITE]);
            if (statusCloseWrite == -1)
            {
                printError("Error in close child -write Descriptor", __LINE__);
            }
            break;
        }
        case M_WRITE:
        {
            // pipe is in read mode from childs perspective
            int statusCloseWrite = close(fd[M_WRITE]);
            if (statusCloseWrite == -1)
            {
                printError("Error in close child - write Descriptor", __LINE__);
            }

            // redirect the stdin to the the read input of the pipe in order to write to the pipe
            if (fd[M_READ] != STDIN_FILENO)
            {
                int statusDupRead = dup2(fd[M_READ], STDIN_FILENO);
                if (statusDupRead == -1)
                {
                    // close the childs read pipe end in chase the dup failed.
                    close(M_READ);
                    printError("Error in duplicating stdin", __LINE__);
                }
            }
            // After redirection close the filedescriptor fd write
            int statusCloseRead = close(fd[M_READ]);
            if (statusCloseRead == -1)
            {
                printError("Error in close child - read Descriptor", __LINE__);
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

/*!
 * @brief Check command and type if are valid
 * @param command const char, the command to be send to the shell
 * @param type const char, the mode in witch the shell operates
 * @return isValid, VALID in case the command and type are valid, INVALID in case one of them are not correct.
 */
static isValid commandCheck(const char *command, const char *type)
{
    // test if on of the parameters is NULL
    if ((command == NULL) || (type == NULL))
    {
        errno = EINVAL;
        return INVALID;
    }

    //first do the correct type check. type is only 1 element long and either 'w' or 'r'
    if ((type[0] != 'w' && type[0] != 'r') || type[1] != 0)
    {
        printError("Type check wrong", __LINE__);
        // set errno to invalid operation
        errno = EINVAL;
        return INVALID;
    }

    return VALID;


}