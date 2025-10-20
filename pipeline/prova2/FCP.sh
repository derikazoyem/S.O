#!/bin/sh
#Soluzione dell'esame del 6 Settembre 2024
# ATTENZIONE IL FILE FCP.sh DEVE ESSERE UN FILE IN CUI IL DIRITTO DI ESECUZIONE ALMENO PER L
'UTENTE RISULTA SETTATO!

#controllo numero parametri: si devono passare Q+1 con Q >= 2
case $# in
0|1|2) 	echo Errore: numero parametri $# quindi pochi parametri. Usage is $0 G F1 F2 ...
       	exit 1;;
*)    	echo DEBUG-OK:da qui in poi proseguiamo con $# parametri;;
esac

#controllo primo argomento
case $1 in
        /*) if test ! -d $1 -o ! -x $1
            then echo $1 non directory o non eseguibile; exit 2
            fi;;
        *) echo Nome directory $1 non assoluto; exit 3;;
esac
G=$1

#elimino il primo parametro per avere solo i nomi di file e quindi calcolarne la meta' oltre che procedere con i controlli che i nomi di file siano relativi semplici
shift
Q=$#
echo DEBUG-Il numero di nomi di file passati uguale a $Q
#In realta' la variabile Q non serve, ma e' stata introdotta per maggior leggibilita'
M=`expr $Q / 2`
echo DEBUG-Si devono trovare almeno $M nomi di file 
Files= #variabile usata per controllo sui nomi

#controllo su nomi relativi degli altri Q parametri 
for F in $*
do
	case $F in
        */*) echo Nome file $F non relativo semplice; exit 4;; 
	#N.B. Questo controllo e' stato aggiunto per sicurezza, ma non e' stato segnato come errore se mancante nelle soluzioni consegnate dagli studenti!
        *)   for i in $Files	#controlliamo che il nome corrente non sia gia' presente
	     do
		if test $F = $i
		then
	     		echo Errore nome semplice $i gia\' presente; exit 5
		fi
   	     done
	     Files="$Files $F";; #altrimenti lo salviamo
	esac
done

PATH=`pwd`:$PATH
export PATH

#chiamiamo la parte ricorsiva passando anche il numero minimo di nomi di file
FCR.sh $G $M $Files	#N.B. Se l'invocazione NON presenta nomi ripetuti il valore di $Files e $* saranno uguali; se non viene effettuato il controllo che e' stato aggiunto nella invocazione bisogna chiaramente usare $*
#NOTA BENE: il calcolo di M poteva anche essere fatto nel file comandi ricorsivo: farlo pero' nel file principale e' meglio perche' viene fatto una sola volta, mentre farlo nel file ricorsivo ha come effetto che viene fatto tutte le volte che si va in ricorsione, ma il valore calcolato sara' comunque sempre lo stesso!
