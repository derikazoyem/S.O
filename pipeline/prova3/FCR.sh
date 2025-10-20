#!/bin/sh
#file comandi ricorsivo che le directory che contengono almeno un file con dimensioni in linee maggiore di X

cd $1
X=$2
#ora dobbiamo di nuovo usare shift due volte per eliminare il nome della gerarchia e il numero (dopo averlo salvato) e avere solo i caratteri; NON serve salvare il primo parametro!
shift
shift
files=	#ci serve per raccogliere i nomi dei file leggibili NON vuoti

for F in *	#F variabile indicata dal testo
do
	if test -f $F -a -r $F	#se troviamo un file leggibile
	then 
		if test `wc -l < $F` -ge $X	#controlliamo il numero di linee
		then 
			files="$files $F"	#salviamo il nome del file con le caratteristiche richieste
		fi
	fi 
done

echo DEBUG-valore di $files per dir `pwd`

#se si trova almeno un file
if test "$files"
then
	echo TROVATA dir `pwd` che ha almeno un file con lunghezza in linee maggiore o uguale e $X
	#per ogni file trovato, dobbiamo usare di nuovo la variabile F e chiamare la parte C
	for F in $files
	do
		echo DEBUG-CHIAMO PARTE C con file $F e stringhe $*
		14Feb24 $F $*
	done
else
	echo DEBUG-NESSUN file in `pwd` che soddisfa le specifiche e quindi niente chiamata parte C
fi 

for i in *
do
	if test -d $i -a -x $i
	then
		#echo RICORSIONE in `pwd`/$i
		$0 `pwd`/$i $X $*
    	fi
done
