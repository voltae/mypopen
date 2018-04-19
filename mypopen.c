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

// --------------------------------------------------------------- defines --

// -------------------------------------------------------------- typedefs --

// --------------------------------------------------------------- globals --

// ------------------------------------------------------------- functions --

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

    // creating a new child process

    // set errno to 0 to protocol an error
    errno = 0;

    // create an integer for the return value of the fork call
    pid_t pid = 0;

    pid = fork();

    // caputer an error if present
    if (pid < 0)
    {
        error(1, errno, "Error occured during fork");
    }

    printf("The pid of the process is: %d!\n", pid);

    // get the pid of the current process

    printf("Get the pid of the child process: %d\n", getpid());
    printf("Get the pid of the parent process: %d\n", getppid());
    return 0;
}