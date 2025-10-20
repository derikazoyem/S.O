#!/bin/sh
# ATTENZIONE IL FILE FCR.sh DEVE ESSERE UN FILE IN CUI IL DIRITTO DI ESECUZIONE ALMENO PER L'UTENTE RISULTA SETTATO!

#entro nella directory
cd $1
#il secondo parametro e' il numero minimo di nomi di file da trovare
M=$2

#elimino i primi due parametri
shift 
shift
#azzero la variabile che serve per il conteggio dei file  
cont=0
#variabile che serve per memorizzare i nomi dei file trovati per poi passarli alla parte C
files=

for i in $* #per tutti i nomi dei file passati
do
#per ogni nome di file lo cerchiamo nella directory corrente
        if test -f $i 	#N.B. il testo parla solo di cercare file e non dice nulla se devono essere leggibili o meno e quindi il controllo sulla leggibilita' NON si deve fare!
        then
		#se lo troviamo aggiorniamo il contatore e memorizziamo il nome relativo semplice 
		cont=`expr $cont + 1`
		files="$files $i"
        fi
done
                 
if test $cont -ge $M 
then
#abbiamo trovato una directory giusta dato che contiene almeno la meta' dei nomi passati!  
#quindi, stampiamo il suo nome su standard output
       	echo Trovata directory giusta `pwd` 
       	#inseriamo una stampa di debugging
       	echo DEBUG-Chiamiamo parte C con $files
	6Set23 $files
fi

for i in *       
do
        if test -d $i -a -x $i
        then 
		#chiamata ricorsiva
		$0 `pwd`/$i $M $*
        fi
done
