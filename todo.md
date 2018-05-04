# mypopen

## done
* All Tests pass
* header file
* implementing the test suite wit the current code
* system signaled __broken pipe__ in write process. This was due the child routine was called in the wrong mode. the write descriptor of the child was close, but it had to write.
* Delete the wait for child routine in the parent process, it seems to work without it. int them mypclose routine close fist the filestream and wait then for the child. This forces the child process to terminate.
* If fork fails it must _exit(EXIT_FAILURE) ont another errorcode. In our case this was _exit(127).
* if the file descriptor of the child process is already the same as the STDOUT_FILENO, it cannot close this file descriptor. This call was outside of the if clause, so it was called, even do it was the same.
* int the mypclose routine first is to check whether the parameter are correct:
    * the incoming file stream must exist.
    * the incoming file stream must be match with the stored one, i.e must be created with mypopen.
## memo
### mypopen()
* the function works with two file-global (static) variables:
    * a filepointer, that stores the current filestream
    * a pid_t integer tat stores the process number of the child process.

myopen creates first does a series of error checks. 
* first does a filepointer and therefore a current process exist, -> YES terminate the program with errno = EAGAIN. It should run only one child proces at a time.
* are the parameter correct?
* create a pipe, and check if the call was successful.
* fork the parent process and distinguish between three cases, from the return value:
   1. the fork failed. close the pipe, set the errno accordingly and return NULL.
   2. the pid is 0, means we are in the child process
   3. the pid greater than 0, we are in the parent process.
     
   in the child process:
   distinguish between two cases. either pipe read or pipe write. (it is seen from parents perspective)
   * close the opposit file descriptor, close()
   * duplicate the open file descriptor to either STDOUT if reading or STDIN if writing, to connect the pipe with the shell. dup2()
   * close the not used filedescriptor.
   * execute the command with execl
   
   in the parent process:
   * close the not used pipe ends (file descriptors). In case reading, the writing end and viceversa
   * create a file stream for either reading or writing with the filedesscriptor, fopen()
   * return the created file stream.
   
### mypclose()
   * error check for the incoming parameters. As described above.
   * important close the incoming filestream before waiting for the child to terminate, if the child is waiting for input. It snds a EOF to the file, that signalizes the child to terminate. 
   * wait for the child in a loop, the loop is not busy waiting.
   * if waiting failed, return -1 in the other case return the eit status of the hild process. 


## todo
all done

## issues
no Tests fail

### mypopen()
* mypopen() muß zunächst eine Pipe einrichten (pipe(2)) 
* dann einen Kindprozeß generieren (fork(2)). 
* Im Kindprozeß ist das richtige Ende der Pipe ("r" oder "w") mit stdin bzw. stdout zu assoziieren (dup2(2)) und das im 1. Argument spezifizierte Kommando auszuführen (execl(3) oder execv(3)). Verwenden Sie - wie die Funktion popen(3) - zum Ausführen des Kommandos die Shell sh(1). 
* Als letztes muß mypopen() von einem Filedeskriptor einen passenden FILE * mit fdopen(3) erzeugen.

### mypclose()
* Bei Aufruf von mypclose() soll der aufrufende Prozeß auf die Terminierung des Kindprozesses warten (waitpid(2)). 
* Zur Vereinfachung soll immer nur höchstens eine Pipe mit mypopen() geöffnet werden können. Stellen Sie dies in ihrer Implementierung sicher.

Signalbehandlung ist für dieses Beispiel nicht notwendig.

### popentest.c
todo: replace the return arguments popen and pclose with the own implementation mypopen and mypclose


## explain the current code
###
library function

### testing code
* im child process ist an erster stelle ein fflush um stin und stdout buffer zu leeren.
* der file-deskriptor stdin wird umgelegt auf eine temporäre variable. daher ist alles, was nun daher kommt umgeleitet
    auf `fileDescrDest` das ist der filedescr des geöffneten files.
    
    