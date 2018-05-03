# mypopen

## done

* header file
* implementing the test suite wit the current code
* system meldet __broken pipe__ wenn Schreibprozess. (Child process im falschen mode aufgerufen)
## todo
Delete the wait for chil routine in the parent process, it seems to work without it.


## issues
Test fail on
* 10
* 12
* 13
* 14
* 17
* 22
* 23
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
    
    