//*
// @file mypopen
// Betriebssysteme mypopen/mypclose File.
// Beispiel 0
//
// @author Valentin Platzgummer <ic17b096@technikum-wien.at>
// @author Lara Kammerer <ic17xxxx@technikum-wien.at>
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
#include <unistd.h> // for fork()
#include <sys/types.h> // for getpid() and getppid()
#include <sys/wait.h> // for waitpid

// --------------------------------------------------------------- defines --

#define SUCCESS 0
#define FAIL 1

// -------------------------------------------------------------- typedefs --

// --------------------------------------------------------------- globals --

static int counter;

// ------------------------------------------------------------- functions --

int createChildProcess ();
int createChildProcessRecursivly();

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


int main() {


    int returnValue = createChildProcess();
    if (returnValue < 0)
    {
        error(1, errno, "Error during fork");
    }

    returnValue = createChildProcessRecursivly();

    return 0;
}

int createChildProcess () {

    // creating a new child process

    // set errno to 0 to protocol an error
    errno = 0;

    // create an integer for the return value of the fork call
    pid_t pid = 0;

    pid = fork();

    // capture an error if present
    if (pid < 0)
    {
        perror("Error during fork");
        return FAIL;
    }


    printf("The pid of the process is: %d!\n", pid);

    // get the pid of the current process, the main method get executed twice, why?
    // System call fork() is used to create processes. It takes no arguments and returns a process ID.
    // The purpose of fork() is to create a new process, which becomes the child process of the caller.
    // After a new child process is created, both processes will execute the next instruction following the fork() system call.
    // Therefore, we have to distinguish the parent from the child. This can be done by testing the returned value of fork():


    // distinguish between child and parent-prcess. parentprocess has the pid 0
    if (pid != 0)
    {
        printf("Get the pid of the child process: %d\n", getpid());
        printf("Get the pid of the parent process: %d\n", getppid());


    }
    return SUCCESS;

}

int createChildProcessRecursivly()
{
    // creating a new child process


    // set errno to 0 to protocol an error
    errno = 0;

    // create an integer for the return value of the fork call
    pid_t pid = 0;

    pid = fork();

    // waiting for termination of child process, only if we are in the parent process
    if (pid == 0)
    {
        // get the child pid number
        pid_t childPid = getpid();

        if ((waitpid(childPid, 1, 0) < 0))
        {
            perror("Error in waiting for pid");
        }
    }



    // caputer an error if present
    if (pid < 0)
    {
        perror("Error during fork");
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
    return SUCCESS;
}
