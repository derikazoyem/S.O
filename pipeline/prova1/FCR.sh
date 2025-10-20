#!/bin/sh
#FCR.sh
#file comandi ricorsivo che scrive il nome dei file trovati insieme con la lunghezza in linee sul file temporaneo il cui nome e' passato come terzo argomento
# ATTENZIONE IL FILE FCR.sh DEVE ESSERE UN FILE IN CUI IL DIRITTO DI ESECUZIONE ALMENO PER L'UTENTE RISULTA SETTATO!

cd $1
#la variabile NR ci serve per il numero di linee dei file
NR=
#per prima cosa azzeriamo il conteggio dei file che soddisfano la specifica
N=0	#NOME STABILITO DAL TESTO

for F in *	#F nome indicato nel testo
do
        #controlliamo solo i nomi dei file leggibili, come richiesto dal testo
        if test -f $F -a -r $F
        then
                #calcoliamo il numero di linee
                NR=`wc -l < $F`
		#controlliamo se il numero delle linee e' maggiore o uguale a X ($2), ma strettamente minore di 255
                if test $NR -ge $2 -a $NR -lt 255 
                        then
                        #abbiamo trovato un file che soddisfa le specifiche 
                	echo `pwd`/$F >> $3  #salviamo il nome assoluto nel file temporaneo
                	echo $NR >> $3  #salviamo anche la dimensione in linee
			#N.B. si poteva anche usare un solo echo e in quel caso il numero di linee ricavato nello script principale non andava diviso per due!
                        N=`expr $N + 1` #incrementiamo il numero di file trovati
                fi
        fi
done

#se ho trovato almeno un file
if test $N -ge 1
then
  echo TROVATA DIRECTORY `pwd` #riportiamo su standard output il nome assoluto della directory
fi

for i in *
do
        if test -d $i -a -x $i
        then
                #chiamata ricorsiva cui passiamo come primo parametro il nome assoluto del direttorio
                FCR.sh `pwd`/$i $2 $3
        fi
done
