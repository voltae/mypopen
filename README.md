# mypopen
Implementation of the 2nd Assigment - Operation Systems
Create a library with 2 functions myopen() mypclose(), which behave like popen(3) and pclose(3).
## mypopen()
1. create pipe: pipe()
2. fork the current process:    fork()
3. close the not used file descriptors: close()
4. in the parent process create a file pointer fdopen()
5. in the child proces duplicate the file descriptor to the STDIN or STOUT to connect the pipe with the shell. dup2()
6. in the child process execute the command: execl()
7. the parent returns the file pointer

##mypclose()
1. Parameter check
2. close the incoming file stream, fclose()
3. wait for the child, waidpid()
4. return the exit status




