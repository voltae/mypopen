//*
// @file mypopen
// Betriebssysteme mypopen/mypclose File.
// Beispiel 0
//
// @author Valentin Platzgummer <ic17b096@technikum-wien.at>
// @author Lara Kammerer <ic17b001@technikum-wien.at>
// @date 2018/04/19
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
/// 0 for readd and 1 for write
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


// -------------------------------------------------------------- typedefs --

// --------------------------------------------------------------- globals --

/// @var pid process id number for the current child process, needed because mypclose need to wait for this process
static pid_t pid;

static int counter;
const char filenameSource[9] = "test.txt";
const char filenameDest[14] = "copy_text.txt";

// ------------------------------------------------------------- functions --
void openPipe(void);
static void printError (const char *errorMessage);
static FILE * ParentPipeStream (int modus, int fd[]);
static FILE * ChildPipeStream (int modus, int fd[]);

static int createChildProcess ();
static void createChildProcessRecursivly();

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
static void usage (void)
{
    printError("Usage: mypopen (command, type)");
}


/// @brief simplified implementation of the library command popen
/// @param The  command argument is a pointer to a null-terminated string containing a shell command line.  This command is passed to /bin/sh using  the -c  flag;  interpretation, if any, is performed by the shell.
/// @param type The type argument is a pointer to a null-terminated string  which  must  contain either the letter 'r' for reading or the letter 'w' for writing.
extern FILE *mypopen (const char *command, const char *type)
{
    // todo: first do the correct type check.


    // change the mode to the enum created above


    // define an array for the file-descriptors of the pipe
    int fd[2];

    // create a new pipe for the program with error checking
    if (pipe(fd) -1)
    {
        printError("Error during create pipe");
    }

    // get an integer rapresenting the mode


    // fork the current process
    // todo: it should be open only one pipe open. check this!

    // Define the Filepointer for the to processes
    FILE *fp_parent_process = NULL;
    FILE *fp_child_process = NULL;

    // allocate a buffer for the command send o the child process
    char commandBuffer[BUFFERSIZE];
    switch (pid = fork())
    {
        case -1:
            printError("Error in fork process");
            break;

            // we are in the chid proces
        case 0:
        {
            // if type is "r" the pipe is in read modus, means parent is reading, child is writing
            if (type == 'r')
            {
                // pipe is in write mode from childs perspective
                fp_child_process = ChildPipeStream(M_WRITE, fd);



            }
                // pipe is in read mode from childs perspective
            else if (type == 'w')
            {

                fp_child_process = ChildPipeStream(M_READ, fd);

            }

                // default path, should not happen
            else
            {
                assert(0);
            }
            execl(EXECPATH, EXECSHELL, EXECCOMM, command);
        }

            // we are in the parent process
        default:
        {
            // if type is "r" the pipe is in read modus from parents perspective
            if (type == "r") {
                fp_parent_process = ParentPipeStream(M_READ, fd);
            }
            else if ( type == "w")
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

}


int main() {


    openPipe();
    // int returnValue = createChildProcess();
    // if (returnValue < 0)
    // {
    //    error(1, errno, "Error during fork");
    //}

    // returnValue = createChildProcessRecursivly();
    // returnValue = createChildProcess();

    return 0;
}

/// @brief configure the parent process
///
/// @param modus ste the parent either to read 0 or write 1
/// @param fd[] array of file descriptors
///
/// @return file Pointer to stream on sucess, null on failure
/// @error errno is set, function leaves with exit failure
static FILE * ParentPipeStream (int modus, int fd[])
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
static FILE * ChildPipeStream (int modus, int fd[])
{
    errno = 0;
    FILE *childStream = NULL;
    switch (modus)
    {
        // child process in read modus
        case M_READ:
        {
            if (close(fd[M_WRITE] == -1))
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
            if (close(fd[M_READ] == -1))
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
int createChildProcess () {

    // creating a new child process

    // set errno to 0 to protocol an error
    errno = 0;

    // create an integer for the return value of the fork call
    pid_t pid = 0;
    int status;

    pid = fork();

    // capture an error if present
    if (pid < 0)
    {
        printError("Error during fork");
    }


    printf("The pid of the process is: %d!\n", pid);

    // get the pid of the current process, the main method get executed twice, why?
    // System call fork() is used to create processes. It takes no arguments and returns a process ID.
    // The purpose of fork() is to create a new process, which becomes the child process of the caller.
    // After a new child process is created, both processes will execute the next instruction following the fork() system call.
    // Therefore, we have to distinguish the parent from the child. This can be done by testing the returned value of fork():


    // distinguish between child and parent-process. parent process has the pid 0
    if (pid != 0)
    {
        printf("Get the pid of the child process: %d\n", getpid());
        printf("Get the pid of the parent process: %d\n", getppid());

        waitpid(pid, &status, 0);
        printf("Waitpid, Status :%d", status);

    }
    exit(0);

}



void openPipe() {
    // Define the array to store the filedescriptors for the pipe
    int fd[2];
    // Define an int to store the pid number of the forked child process
    pid_t pid = 0;

    // create the pipe and check if an error occurred
    if (pipe(fd) == -1)
    {
        printError("Error in creating the pipe");
    }



    switch (pid = fork()) {

        case -1:
            printError("error in fork");
            break;

            //we are in child process
        case 0: {

            // Clean out the standard in and standard out buffer
            fflush(stdin);
            fflush(stdout);


            // change the filedescriptor fd to a file pointer in order to read the pipe
            FILE *fp_read = fdopen(fd[M_READ], "r");

            // if the filepointer is NULL, there was an error during fdopen
            if (fp_read == NULL)
            {
                printError("Error during fdopen child file pointer");
            }

            // we close the write end of the pipe, since we want to read from the pipe-------why the write end if we want to write from parent to child? or do we want to read?-------------
            close(fd[M_WRITE]);

            // get the filepointer from the destination File
            FILE *fp_Dest = fopen(filenameDest, "w+"); //wozu �bergibst du einen mode? VALENTIN: O_CREATE legt ein neues File an falls es nicht existiert mit den File Permissions 644

            printf("Child process\n");
            if (fp_Dest == NULL) {
                printError("Error in open file ");
            }

            // Redirect the stdin to the pipe in order to read from pipe and check error
            if (dup2(fd[M_READ], STDIN_FILENO) == -1)
            {
                printError("Error in duplicating stdin");
            }

            // allocate a buffer to read
            char buf[BUFFERSIZE];

            // read the file from the pipe and check if error occurred
            if (fgets(buf, sizeof(buf), fp_read) == NULL)
            {
                if (ferror(fp_read))
                {
                    printError("An error occurred while reading from pipe");
                }
            }

            printf("I read the pipe with the text: %s\n", buf);

            // Close the read end of the pipe, all read and check for error
            if (close(fd[M_READ]) == -1)
            {
                printError("Error in close pipe");
            }

            // Write the read text to the destination file
            if (fputs(buf, fp_Dest) == EOF)
            {
                printError("Error in writing to file");
            }



            // close desitnation file
            fprintf(stdout, "File read from parent: %s\n", buf); //zum anschauen ob es geklappt hat w�rd ich eher schaun was im beliebiges_file.txt drin steht bzw im fileDescrDest------------



            // Close the open File
            fclose(fp_Dest);

            // execute the cat command
            //region cat the current file
            fprintf(stdout, "------- cat statement begin -------\n");
            if (execl("/bin/cat", "cat",filenameDest , NULL) < 0)
            {
                printError("error in execl function");
            }
            //endregion
            fprintf(stdout, "------- cat statement end -------\n");
            // if execl fails, this line is executed.
            assert(0);




            break;
        }

            // pid greater than 0, this is the parent process
        default:
        {

            int fileDescriptorSource;
            printf("Parent Process!\n");

            // Change the filedescriptor of the pipe in parent to a file pointer
            FILE *fp_pipe_write = fdopen(fd[M_WRITE], "w");
            if (fp_pipe_write == NULL)
            {
                printError("Error in parent. Pipe can not write");
            }

            // get the fileDescriptorSource from the source file
            FILE *fp_source = fopen(filenameSource, "r"); //bei den flags bin ich mir �brigens nicht sicher..da w�rs toll wenn du die mit mir nochmal durchgehn k�nntest--- VAL: ja
            if (fp_source == NULL) {
                printError("Error in readfile");
            }
            // we close the read end of the pipe, we want to write to the child process---------think it's the false end we're closing...------------------------------
            // VAL: So scheint es aber zu funktionieren. Außerdem "if the parent wands to send data to the child, it shoulds close fd[0], and the child should close fd[1]. GNU Creating Pipes in C
            close(fd[M_READ]);


            // This is the Buffer for the File
            char buf[BUFFERSIZE];
            // read Buffersize  bytes from the source file, as long as the file is entirely read
            while (fgets(buf, sizeof(buf), fp_source) != NULL)
            {
                if (ferror(fp_source))
                {
                    printError("Error in write to source file");
                }
            }
            printf("From File: %s\n", buf);


            // now write the puffer to the pipe
            while (fputs(buf, fp_pipe_write) != EOF)
            {
                if (ferror(fp_pipe_write))
                {
                    printError("Error in writing from parent to pipe");
                }
            }
            // close the write end of the pipe
            close(fd[M_WRITE]);
            fprintf(stdout, "File written to child: %s\n", buf);

            // close the open Source File
            close(fileDescriptorSource);

            break;
        }
    }
}

/*
void createChildProcessRecursivly()
{
    // creating a new child process


    // set errno to 0 to protocol an error
    errno = 0;

    // create an integer for the return value of the fork call
    pid_t pid = 0;

    pid = fork();
    int status;

    // waiting for termination of child process, only if we are in the parent process
    if (pid == 0)
    {
        // get the child pid number
        pid_t childPid = getpid();


        if ((waitpid(childPid, &status, 0) < 0))
        {
            printError("Error in waiting for pid");
        }
        printf("status: %d", status);
    }



    // capture an error if present
    if (pid < 0)
    {
        printError("Error during fork");
        return FAIL;
    }


    printf("The pid of the process is: %d!\n", pid);

    // counter has to be a global variable, local get set always after a child process get forked!
    printf("Number of processes: %d\n", counter++);


    // distinguish between child and parent-process. parentprocess has the pid 0
    if (pid != 0)
    {
        printf("Get the pid of the child process: %d\n", getpid());
        printf("Get the pid of the parent process: %d\n", getppid());


        // make recursive call to create child process
        createChildProcessRecursivly();

    }
}*/
