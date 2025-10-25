#!/bin/sh
#file comandi ricorsivo che identifica un certo livello di una gerarchia, supposto che esista

cd $1
#il primo livello verra' contato come livello 1
conta=`expr $3 + 1`  #nota bene questa variabile conta e' una variabile conta LOCALE a questo script
files=	#ci serve per raccogliere i nomi dei file leggibili NON vuoti

#verifichiamo se $conta e' pari
ris=`expr $conta % 2` 

#echo conta uguale $conta e ris uguale $ris

if test $ris -eq 0 #siamo ad un livello pari e quindi bisogna raccogliere i nomi di tutti i file leggibili con lunghezza in linee uguale a $2
then
	echo TROVATA directory `pwd` che si trova a livello PARI $conta
	for F in *	#F variabile indicata dal testo
	do
		if test -f $F -a -r $F
		then 
			if test `wc -l < $F` -eq $2
			then 
				files="$files $F"
			fi
		fi 
	done
	#se ho trovato almeno un file
	if test "$files"
	then
		echo DEBUG-CHIAMO PARTE C con file $files
		17Gen24 $files
	else
	        echo NESSUN file che soddisfa le specifiche e quindi niente chiamata parte C
        fi
fi 

for i in *
do
	if test -d $i -a -x $i
	then
		#echo RICORSIONE in `pwd`/$i  con terzo paramentro uguale a $conta
		$0 `pwd`/$i $2 $conta
    	fi
done
