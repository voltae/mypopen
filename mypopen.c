//*
// @file mypopen
// Betriebssysteme mypopen/mypclose File.
// Beispiel 2
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

#include "mypopen.h"

// --------------------------------------------------------------- defines --
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

// --------------------------------------------------------------- globals --

/*! @var pid process id number for the current child process, needed because mypclose need to wait for this process */
static pid_t childpid = -1;

/*!
 * @brief local varialbe points to the current open filestream
 */
static FILE *filepointer = NULL;

// ------------------------------------------------------------- functions --

/*!
 * @brief check function input against valitity
 * @param command the command to execute
 * @param type either 'r' or 'w', read or write
 * @return isValid, an enum returning if valid or not
 */
static isValid commandCheck(const char *command, const char *type);

// check if char is in string
/*static int isCharInString(const char * string);*/

/*!
 * @brief Creates the parent side of the pipe
 * @param modus defines the mode in witch the pipe is in, either 'M_READ' or 'M_WRITE'
 * @param fd an integer array, holds the number of the file descriptor created by the crate pipe process
 * @return a FILE Pointer to the pipe, to witch can be read or written
 */
static FILE *ParentPipeStream(int modus, int fd[]);

/*!
 * @brief Creates the child side of the pipe
 * @param modus defines the mode in witch the pipe is in, either 'M_READ' or 'M_WRITE'
 * @param fd an integer array, holds the number of the file descriptor created by the crate pipe process
 */
static void ChildPipeStream(int modus, int fd[]);

/*!
* @brief Implementation of a simplifed popen() library function
*
* This is the main entry point for any C program.
*
* @param command argument is a pointer to a null-terminated string containing a shell command line. This command is passed to /bin/sh using  the -c  flag;  interpretation, if any, is performed by the shell.
* @param type a pointer to a null-terminated string  which  must  contain either the letter 'r' for reading or the letter 'w' for writing.
* @return FILE Pointer connecting parent and shell in success case, NULL in case it fails
*/
extern FILE *mypopen(const char *command, const char *type)
{

    // define an array for the file-descriptors of the pipe
    int fd[2];

    // a filepointer exists, means there is a current process running
    if (filepointer != NULL)
    {
        errno = EAGAIN;
        return NULL;
    }

    // Check if the given arguments are valid, errno is set in the function
    if (commandCheck(command, type) == INVALID)
    {
        return NULL;
    }

    // set errno to 0 to capture the right error message
    errno = 0;

    // create a new pipe for the program with error checking
    if (pipe(fd) == -1)
    {
        errno = ECHILD;
        return NULL;
    }

    // fork the current process. It should always be open only one pipe
    childpid = fork();

    switch (childpid)
    {
        case -1:
        {
            // store errno local to not get overwritten by close
            const int err = errno;
            // Closing the pipes end if the fork process failed
            close(fd[M_READ]);
            close(fd[M_WRITE]);
            errno = err;
            return NULL;
            break;
        }

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

            // if we get to this point, an error occurred,
            _exit(127);
            break;
        }

            // we are in the parent process
        default:
        {
            // if type is "r" the pipe is in read modus from parents perspective
            if (type[0] == 'r')
            {
                filepointer = ParentPipeStream(M_READ, fd);
            }
            else if (type[0] == 'w')
            {
                filepointer = ParentPipeStream(M_WRITE, fd);
            }
                // default path, should not happen
            else
            {
                assert(0);
            }
            break;
        }
    }

    return filepointer;
}

/// @brief
/// @returns the exit status of the wait system call
/*!
 * @brief The mypclose() function waits for the associated process to terminate and returns the exit status of the command as returned by wait(2).
 * @param stream a filestream created by a previous mypopen command,
 * @return the exit status of the terminated child or if the exit status failed, -1
 */
extern int mypclose(FILE *stream)
{

    /* define the needed variables */
    int status = 0;
    pid_t waitedChild;

    // If mypclose is called without any actual child open
    if (filepointer == NULL)
    {
        errno = ECHILD;
        return INVALID;
    }

    // Check if the incoming stream is created with mypopen, if stream is NULL it fails either
    if (stream != filepointer)
    {
        errno = EINVAL;
        return INVALID;
    }
    // close file Pointer, flushes the stream, and terminate the child process
    if (fclose(stream) == EOF)
    {
        // reset global variables filepointer and childpid, errno is set by fclose
        filepointer = NULL;
        childpid = -1;
        return INVALID;
    }

    /* wait for terminating properly the child process */
    do
    {
        waitedChild = waitpid(childpid, &status, 0);
    } while (waitedChild == -1 && errno == EINTR);

    /* Reset all global variables for the next process*/
    childpid = -1;
    filepointer = NULL;

    fprintf(stderr, "STATUS: %d\n", WIFEXITED(status));

    if (waitedChild == -1) // errno is set by widpid
    {
        return INVALID;
    }
    if (WIFEXITED(status))
    {
        return WEXITSTATUS(status);
    }

/*if an error occured, set the errno appropriately */
    errno = ECHILD;
    return INVALID;
}

/// @brief configure the parent process
///
/// @param modus the parent either to read 0 or write 1
/// @param fd[] array of file descriptors
///
/// @return file Pointer to stream on success, null on failure
/// @error errno is set appropriately
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
                exit(EXIT_FAILURE);
            }
            // try to open a file stream to read the pipe
            parentStream = fdopen(fd[M_READ], "r");
            if (parentStream == NULL)
            {
                // close the read end of the parents pipe
                (void) close(fd[M_READ]);
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
                exit(EXIT_FAILURE); // errno is set by close
            }
            // try to open a file stream to read the
            parentStream = fdopen(fd[M_WRITE], "w");
            if (parentStream == NULL)
            {
                // close writes end of the parents pipe
                (void) close(fd[M_WRITE]); // errno is set by close
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

    return parentStream;
}

/// @brief Create the file stream for the child process
/// @param modus M_READ (0) for reading pipe, M_WRITE (1) for write to pipe
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
            (void) close(fd[M_READ]); // errno get set by close

            // Redirect the stdout to the pipe in order to read from pipe and check error
            if (fd[M_WRITE] != STDOUT_FILENO)
            {
                int statusDupWrite = dup2(fd[M_WRITE], STDOUT_FILENO);
                if (statusDupWrite == -1)
                {
                    // close the childs write pipe end in chase the dup failed.
                    (void) close(M_WRITE);
                    _exit(EXIT_FAILURE); // Exit with error
                }

                // After redirection close the filedescriptor fd write, errno get set by close
                (void) close(fd[M_WRITE]);
            }


            break;
        }
        case M_WRITE:
        {
            // pipe is in read mode from childs perspective
            (void) close(fd[M_WRITE]); // errno get set by close


            // redirect the stdin to the the read input of the pipe in order to write to the pipe
            if (fd[M_READ] != STDIN_FILENO)
            {
                int statusDupRead = dup2(fd[M_READ], STDIN_FILENO);
                if (statusDupRead == -1)
                {
                    // close the childs read pipe end in chase the dup failed.
                    (void) close(M_READ); // close pipe on error
                    _exit(EXIT_FAILURE); // and leave the function with error // default was EXIT_FAILURE
                }
                // After redirection close the filedescriptor fd write
                (void) close(fd[M_READ]);
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
        // set errno to invalid operation
        errno = EINVAL;
        return INVALID;
    }

    return VALID;
}
