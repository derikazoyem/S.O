#!/bin/sh
#Soluzione dell'appello del 10 Luglio 2024
#ATTENZIONE IL FILE FCP.sh DEVE ESSERE UN FILE IN CUI IL DIRITTO DI ESECUZIONE ALMENO PER L'UTENTE RISULTA SETTATO!
#versione che usa un file temporaneo e usa FCR.sh per tutte le fasi

case $# in
0|1|2)	echo Errore: numero parametri is $# quindi pochi parametri. Usage is $0 numero dirass1 dirass2 ...
	exit 1;;
*) 	echo OK: da qui in poi proseguiamo con $# parametri ;;
esac

#Controllo che primo parametro sia un numero strettamente positivo e strettamente maggiore di 20
case $1 in
*[!0-9]*)       echo Primo parametro $1 non numerico o non positivo
                exit 2;;
*)              if test $1 -eq 0 -o $1 -ge 20 
                then    echo Secondo parametro $2 zero oppure non strettamente minore di 20
                        exit 3
                fi;;
esac

X=$1 #salviamo il numero nella variabile indicata dal testo 
#quindi ora possiamo usare il comando shift
shift

#ora in $* abbiamo solo i nomi delle gerarchie e quindi possiamo fare i controlli sui nomi assoluti e sui direttori in un for
for G 
do
	case $G in
	/*) if test ! -d $G -o ! -x $G
	    then
	    echo $G non direttorio
	    exit 4
	    fi;;
	*)  echo $G non nome assoluto; exit 5;;
	esac
done

#controlli sui parametri finiti possiamo passare alle Q fasi
PATH=`pwd`:$PATH
export PATH
> /tmp/nomiAssoluti #creiamo/azzeriamo il file temporaneo. NOTA BENE: SOLO 1 file temporaneo!

for G	#NOME STABILITO DAL TESTO
do
	echo fase per $G
 	#invochiamo il file comandi ricorsivo con la gerarchia, il numero e il file temporaneo
	FCR.sh $G $X /tmp/nomiAssoluti   
done

#terminate tutte le ricerche ricorsive cioe' le Q fasi
#N.B. Andiamo a contare le linee del file /tmp/nomiAssoluti
NTot=`wc -l < /tmp/nomiAssoluti` 
N=`expr $NTot / 2`	#N.B. Il numero dei file trovati corrisponde alla meta' delle linee dato che sono stati salvate nel file anche le lunghezze in linee dei file trovati (su linee separate). Se non si fossero salvate su linee separate allora NON sarebbe stato necessario dividere per 2! 
echo Il numero di file totali che soddisfano la specifica = $N 
echo "DEBUG-I file sono i seguenti (intervallati dalla dimensione in linee)"
echo `cat /tmp/nomiAssoluti` #usato questo comando invece che il semplice cat /tmp/nomiAssoluti solo per avere una stampa compatta!
#chiamiamo la parte in C
10Lug24 `cat /tmp/nomiAssoluti`

#da ultimo eliminiamo il file temporaneo
rm /tmp/nomiAssoluti
