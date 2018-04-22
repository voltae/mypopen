# mypopen

## done

## todo


## explain the current
* im child process ist an erster stelle ein fflush um stin und stdout buffer zu leeren.
* der file-deskriptor stdin wird umgelegt auf eine temporäre variable. daher ist alles, was nun daher kommt umgeleitet
    auf `fileDescrDest` das ist der filedescr des geöffneten files