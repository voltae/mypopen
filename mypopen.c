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
#include "mpopen.h"




// --------------------------------------------------------------- defines --

#define SUCCESS 0
#define FAIL 1

#define BUFFERSIZE 256      // For read and write files

// -------------------------------------------------------------- typedefs --

// --------------------------------------------------------------- globals --
static int counter;

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
        perror("Error during fork");
        return FAIL;
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


    return SUCCESS;

}

void openPipe(char *filename)
{
    // filedescriptors
    int fd[2];
    // pid number
    pid_t pid = 0;
    int resultPipe = pipe (fd);

    printf("resultPipe: %d\n", resultPipe);

    pid = fork();

    if (pid < 0)
    {
        perror("error in fork");
        exit(1);
    }
    else if (pid == 0) { //we are in child process
        // get the fileDescriptor from the destination File
         int fileDescrDest = open("beliebiges_file.txt", O_WRONLY | O_CREAT, 0644); //wozu �bergibst du einen mode?---------------------------------------------
        printf("Child process\n");
        if  (fileDescrDest < 0)
        {
            perror("Error in open");
        }
        // we close the write end of the pipe, since we want to read from the pipe-------why the write end if we want to write from parent to child? or do we want to read?-------------
        close(fd[1]);

        // allocate a buffer to read
        char buf[BUFFERSIZE];
        // read the file from the pipe
        int readBytes = read(fd[0], buf, BUFFERSIZE); //wouldn't it be read from the write end of the pipe?-------------------------------------------------------------
        if (readBytes < 0)
        {
            perror("error in read Bytes");
        }
        int writtenBytes = write(fileDescrDest, buf, strlen(buf)+1); //writes from buf in fileDescrDest
        if (writtenBytes < 0)
        {
            perror("Error in writing bytes");
        }
        else
        {
            fprintf(stdout, "File read from parent: %s\n", buf); //zum anschauen ob es geklappt hat w�rd ich eher schaun was im beliebiges_file.txt drin steht bzw im fileDescrDest------------
            close(fileDescrDest);
        }


    }

    else if (pid > 0) //we are in parent process
    {
        int fileDescriptorSource;
        printf("Parent Process!\n");

        // get the fileDescriptorSource from the source file
        fileDescriptorSource = open("test.txt", O_RDONLY); //bei den flags bin ich mir �brigens nicht sicher..da w�rs toll wenn du die mit mir nochmal durchgehn k�nntest---
        if (fileDescriptorSource < 0)
        {
            perror("Error in readfile");
        }
        // we close the read end of the pipe, we want to write to the child process---------think it's the false end we're closing...------------------------------
        close(fd[0]);

        // This is the Buffer for the File
        char buf[BUFFERSIZE];
        // read 1024 bytes from the file
        int bytesRead = read(fileDescriptorSource, &buf, BUFFERSIZE);
        printf("From File: %s\n", buf);

        // now write the puffer to the pipe
        int writtenBytes = write(fd[1], buf, strlen(buf) +1); //----------------------------...and therefore the false end we're using...-------------------------
        if (writtenBytes < 0)
        {
            perror("error in writebytes to pipe");
        }
        else
        {
            fprintf(stdout, "File written to child: %s\n", buf);
            close(fileDescriptorSource);
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
            perror("Error in waiting for pid");
        }
        printf("status: %d", status);
    }



    // capture an error if present
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
}*/
