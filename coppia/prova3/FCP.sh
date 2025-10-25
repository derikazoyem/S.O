#!/bin/sh
#Soluzione della Prova del 10 Luglio 2019
#File comandi principale da invocare con Cz (singolo car) dirass1 dirass2 ... 
#Viene usato un file temporaneo e FCR.sh e' usato per tutte le fasi
# ATTENZIONE IL FILE FCP.sh DEVE ESSERE UN FILE IN CUI IL DIRITTO DI ESECUZIONE ALMENO PER L'UTENTE RISULTA SETTATO!

#controllo sul numero dei parametri: controllo lasco, ci devono essere almeno un carattere e almeno due directory!
case $# in
0|1|2)   echo Errore: voglio un numero parametri maggiore o uguale a 3, mentre ne hai passati $# - Usage is $0 Cz dirass1 dirass2 ...
         exit 1;;
*)       echo DEBUG-OK: da qui in poi proseguiamo con $# parametri ;;
esac

#Controllo primo parametro $1
case $1 in
	?) echo DEBUG-$1 singolo carattere ;;
	*) echo Errore: $1 non singolo carattere
	   exit 2;;
esac

#salviamo il primo parametro: singolo carattere
Cz=$1

#quindi ora possiamo usare il comando shift per eliminare il primo parametro e quindi il carattere  
shift

#ora in $* abbiamo solo i nomi delle gerarchie e quindi possiamo fare i controlli sui nomi assoluti e sulle directory in un for
for i
do
        case $i in
        /*) if test ! -d $i -o ! -x $i
            then
            	echo Errore: $i non directory o non traversabile
            	exit 4
            fi;;
        *)  echo Errore: $i non nome assoluto
	     exit 5;;
        esac
done

#controlli sui parametri finiti possiamo passare alle W fasi, dopo aver settato il path
PATH=`pwd`:$PATH
export PATH

#azzeriamo/creiamo il file temporaneo
> /tmp/files$$

for i
do
        echo DEBUG-fase per $i
	FCR.sh $i $Cz /tmp/files$$ 
done

#N.B. Andiamo a contare le linee del file /tmp/temp$$ anche se non richiesto e stampiamo tale numero
NF=`wc -l < /tmp/files$$`
echo DEBUG-Il numero totale di file trovati e\' $NF
echo DEBUG-Chiamo la parte in C con `cat /tmp/files$$`
10Lug19 `cat /tmp/files$$` $Cz

#rimuoviamo il file temporaneo
rm /tmp/files$$
