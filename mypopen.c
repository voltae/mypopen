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
/*typedef struct childProcess
{
    pid_t parentpid; // bl�de Frage aber wozu is die? wird nie verwendet ---------------------------------------------------------------------------------------
    pid_t childpid;
    int *fd;
    FILE *filepointer;
} childProcess;*/

// --------------------------------------------------------------- globals --

/*! @var pid process id number for the current child process, needed because mypclose need to wait for this process */
static pid_t childpid = -1;

/*!
 * @brief local varialbe points to the current open filestream
 */
static FILE *filepointer = NULL;

// ------------------------------------------------------------- functions --

static void printError(const char *errorMessage, int lineNumber);

static isValid commandCheck(const char *command, const char *type);

// check if char is in string
/*static int isCharInString(const char * string);*/

static FILE *ParentPipeStream(int modus, int fd[]);

static void ChildPipeStream(int modus, int fd[]);

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
    // no return?---------------------------------------------------------------------------------------------------------------------------------
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

    // define an array for the file-descriptors of the pipe
    int fd[2];

    // a filepointer exists, means there is a current process running
    if (filepointer != NULL)
    {
        errno = EAGAIN;
        return NULL;
    }

    // Check if the given arguments are valid
    if (commandCheck(command, type) == INVALID)
    {
        //errno = EINVAL; unn�tig oder in commandCheck unn�tig--------------------------------------------------------------------------------auskommentiert
        //fprintf(stderr, "Illegal input: %s", strerror(errno)); //should be printed in stderr? ------------------------------------------------------auskommentiert
        return NULL;
    }

    // set errno to 0 to capture the right error message
    errno = 0;

    // create a new pipe for the program with error checking
    if (pipe(fd) == -1)
    {
        //printError("Error in creating pipe", __LINE__); //should be printed in stderr?----------------------------------------------------------------auskommentiert
        errno = ECHILD;
        return NULL;
    }

    // fork the current process. It should always be open only one pipe
    childpid = fork();

    switch (childpid)
    {
        case -1:
            printError("Error in fork process", __LINE__);
            // store errno local to not get overwritten by close
            const int err = errno;
            // Closing the pipes end if the fork process failed
            close(fd[M_READ]);
            close(fd[M_WRITE]);
            errno = err;
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
            execl(EXECPATH, EXECSHELL, EXECCOMM, command,
                  (char *) NULL); //das kann ich auf die schnelle ned kontrolliern --------------------------------------------

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

/// @brief The pclose() function waits for the associated process to terminate and returns the exit status of the command as returned by wait4(2).
/// @returns the exit status of the wait system call
extern int mypclose(FILE *stream)
{

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
    // close file Pointer, and terminate the child process
    if (fclose(stream) == EOF)
    {
        // set stream to NULL
        stream = NULL;
        // reset global variables filepointer and childpid
        filepointer = NULL;
        childpid = -1;
        printError("Error in close file", __LINE__);
        return INVALID;
    }

    int status = 0;
    pid_t waitedChild;

    /* wait for terminating properly the child process */
    do
    {
        waitedChild = waitpid(childpid, &status, 0);
    } while (waitedChild == -1 && errno ==
                                  EINTR); //check once more -----------------!!-----------------------------------------------------------------------

    //printf("Exit status: %d\n", status); -----------------------------------------------------------------------------------------------------auskommentiert
    if (WIFEXITED(status))
    {
        /* reset the global varaibles and return the exit value */
        stream = NULL;
        childpid = -1;
        filepointer = NULL;
        return WEXITSTATUS(status); //so richtig? ------------------------------------------------------------------------------------------------------------
    }

    childpid = -1;
    filepointer = NULL;
    stream = NULL;


    printf("Exit true %d, Exit status: %d\n",
           WIFEXITED(status), WEXITSTATUS(status)
    );


    if (waitedChild == -1)
    {
        printf("waited Child is -1\n");
        stream = NULL;
        errno = ECHILD;
        return INVALID;
    }

/*if an error occured */
    errno = ECHILD;
    printf("14 should reach here\n");

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
                //printError("Error in close write Descriptor", __LINE__); keine andere R�ckmeldung wenn es schiefgeht? ---------------auskommentiert
            }
            // try to open a file stream to read the pipe
            parentStream = fdopen(fd[M_READ], "r");
            if (parentStream == NULL)
            {
                // close the read end of the parents pipe
                close(fd[M_READ]);
                //printError("Error in read pipe", __LINE__); -------------------------------------------------------------------------auskommentiert
                //keine R�ckmeldung f�r close -------------------------------------------------------------------------------------------------------
                return NULL; //unn�tig weil sowieso mit return parentStream NULL returnt w�rde wenn parentStream == NULL ----------------------------
            }
            break;
        }
            // parent process in write modus
        case M_WRITE:
        {
            int statusCloseRead = close(fd[M_READ]);
            if (statusCloseRead == -1)
            {
                //printError("Error in close read Descriptor", __LINE__); keine andere R�ckmeldung wenn es schiefgeht? ---------------auskommentiert
                //printf("read: %d, write: %d\n", fd[M_READ], fd[M_WRITE]); ----------------------------------------------------------auskommentiert
            }
            // try to open a file stream to read the
            parentStream = fdopen(fd[M_WRITE], "w");
            if (parentStream == NULL)
            {
                // close writes end of the parents pipe
                close(fd[M_WRITE]);
                //printError("Error in write pipe", __LINE__); -----------------------------------------------------------------------auskommentiert
                // keine R�ckmeldung wenn close schiefgeht------------------------------------------------------------------------------------------
                return NULL; // unn�tig wenn parentStream == NULL ----------------------------------------------------------------------------------
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
            // pipe is in write mode from childs perspective
            (void) close(fd[M_READ]); // errno get set by close

            // Redirect the stdout to the pipe in order to read from pipe and check error
            if (fd[M_WRITE] !=
                STDOUT_FILENO) //wieso sollte es schon dort liegen? ----------------------------------------------------------VP Defensiv-
            {
                int statusDupWrite = dup2(fd[M_WRITE], STDOUT_FILENO);
                if (statusDupWrite == -1)
                {
                    // close the childs write pipe end in chase the dup failed.
                    (void) close(
                            M_WRITE); //wieso schlie�t dus nur bei einem FAIL? ------------------------------------Bei Fail schließe ich die komplette Pipe---
                    //printError("Error in duplicating stout", __LINE__); -------------------------------------------------------------------auskommentiert
                }
            }
            // After redirection close the filedescriptor fd write, errno get set by close
            (void) close(fd[M_WRITE]);

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
                    close(M_READ); // wieso schlie�t dus nur bei einem FAIL? ------------------------------------------------------------------------------
                    //btw kein error von close gecheckt?----------------------------------------------------------------------------------
                    //printError("Error in duplicating stdin", __LINE__); --------------------------------------------------------------------auskommentiert
                }
            }
            // After redirection close the filedescriptor fd write
            int statusCloseRead = close(fd[M_READ]);
            if (statusCloseRead == -1)
            {
                //printError("Error in close child - read Descriptor", __LINE__); keine andere R�ckmeldung wenn es schiefgeht? ---------------auskommentiert
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
        //printError("Type check wrong", __LINE__);----------------------------------------------------------------------------auskommentiert
        // set errno to invalid operation
        errno = EINVAL;
        return INVALID;
    }

    return VALID;
}
