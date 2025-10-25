#!/bin/sh
#Soluzione dell'esame del 25 Gennaio 2023
#File comandi principale da invocare con dirass stringa1 stringa2

#Controllo sul numero dei parametri: controllo stretto, ci devono essere esattamente 3 parametri!
case $# in
0|1|2) 	echo Errore: voglio 3 parametri, mentre ne hai passati $# quindi pochi parametri - Usage is $0 dirass E1 E2
	exit 1;;
3) 	echo DEBUG-OK numero di parametri = $# ;;
*) 	echo Errore: voglio 3 parametri, mentre ne hai passati $# quindi troppi parametri - Usage is $0 dirass E1 E2
	exit 2;;
esac

#Controllo primo parametro: deve essere un nome assoluto di una directory traversabile
case $1 in
/*) 	if test ! -d $1 -o ! -x $1
	then
		echo Errore: $1 non directory o non attraversabile
		exit 3
	fi;;
*) 	echo Errore: $1 non nome assoluto
	exit 4;;
esac

G=$1 #salviamo il primo parametro (N.B. nella variabile il cui nome viene specificato nel testo)

#quindi ora possiamo usare il comando shift
shift

#ora in $* abbiamo solo le due stringhe e quindi possiamo fare i controlli adeguati
for i 
do
	case $i in
	*/*) echo Errore: nella stringa $i che sara\' usata come estensione non ci puo\' essere uno slash 
	     exit 5;;
	*)   #echo DEBUG-OK $i stringa corretta  
		;;
	esac
done

#solo per scrupolo controlliamo che le due stringhe passate NON siano uguali
if test $1 = $2
then
	echo Errore: le due stringhe $1 e $2 sono uguali e questo non e\' corretto
	exit 6
fi

#controlli sui parametri finiti, quindi settiamo il path 
PATH=`pwd`:$PATH
export PATH

#invochiamo il file comandi ricorsivo con la gerarchia e le due stringhe 
FCR.sh $G $*	#in alternativa invece che $* si poteva scrivere $1 e $2
