#!/bin/sh
#Soluzione della Prova del 14 Febbraio 2024
# ATTENZIONE IL FILE FCP.sh DEVE ESSERE UN FILE IN CUI IL DIRITTO DI ESECUZIONE ALMENO PER L'UTENTE RISULTA SETTATO!

case $# in
0|1|2|3) echo Errore: numero parametri $# quindi pochi parametri. Usage is $0 G X S1 S2 ...
         exit 1;;
*)       echo DEBUG-OK: da qui in poi proseguiamo con $# parametri ;;
esac

#Controllo $1
#dobbiamo controllare che il primo parametro sia una gerarchia
case $1 in
/*) if test ! -d $1 -o ! -x $1
    then
    	echo $1 non directory o non traversabile
        exit 2
    fi;;
*)  echo $1 non nome assoluto; exit 3;;
esac

G=$1	#salviamo la gerarchia nella variabile indicata dal testo

#Controllo $2
#dobbiamo controllare che il secondo parametro sia un numero e sia strettamente maggiore di 0 
case $2 in
*[!0-9]*) echo $2 non numerico o non positivo
	  exit 4;;
*)        if test $2 -eq 0      #sbagliato
          then
          	echo $2 uguale a 0
		exit 5
	  else
	        echo DEBUG-secondo parametro giusto $2
	  fi;;
esac
					
X=$2    #salviamo il numero nella variabile indicata dal testo

#quindi ora possiamo usare il comando shift due volte per eliminare la gerarchia e per eliminare il numero
shift
shift

#ora in $* abbiamo solo le stringhe e quindi possiamo fare i controlli che abbiano almeno due caratteri
for S
do
        case $S in
	?) #se la stringa e' costituita da un solo carattere NON e' una stringa corretta
           echo $S non ha almeno due caratteri
           exit 4;;
        *) echo DEBUG-$S stringa corretta con almeno due caratteri;;
        esac
done

#controlli sui parametri finiti possiamo passare all'unica fase, dopo aver settato il path
PATH=`pwd`:$PATH
export PATH

#invochiamo il file comandi ricorsivo con la gerarchia, il numero e tutte le stringhe passate
FCR.sh $G $X $* 
