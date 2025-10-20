/* Soluzione della parte C del compito del 16 Febbraio 2022 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>

/* definizione del TIPO pipe_t come array di 2 interi */
typedef int pipe_t[2];

/* definizione del TIPO s_occ per le strutture dati, che comporranno gli array dinamici inviati sulle pipe */
typedef struct {
        int Fpid;         	/* pid figlio (campo c1 del testo) */
        long int occ;   	/* numero occorrenze (campo c2 del testo) */
                } s_occ;

int main (int argc, char **argv)
{
   int N;          /* numero di file (NOME STABILITO DAL TESTO!) */
   int pid;        /* pid per fork */
   pipe_t *pipes;  /* array di pipe usate a pipeline da primo figlio, a secondo figlio .... ultimo figlio e poi al padre: ogni processo (a parte il primo) legge dalla pipe i-1 e scrive sulla pipe i. N.B. Nel caso di schema a pipeline e' molto importante che read e write abbiano SUCCESSO! */
   int i,j;        /* indici per i cicli (i NOME STABILITO DAL TESTO!) */
   int fd;         /* file descriptor */
   int pidFiglio, status, ritorno; /* per valore di ritorno figli */
   char Cx, ch;    /* carattere da cercare e carattere letto da linea (ch NOME STABILITO DAL TESTO!) */
   s_occ *cur;     /* array di strutture usate dal figlio corrente (NOME STABILITO DAL TESTO!) e dal padre */
   int nr,nw;           /* variabili per salvare valori di ritorno di read/write da/su pipe */

	/* controllo sul numero di parametri: almeno 2 file e un carattere */
	if (argc < 4)
	{
        	printf("Errore: Necessari almeno 3 parametri per %s e invece argc = %d\n", argv[0] , argc); 
		exit(1);
	}

	/* controlliamo che l'ultimo parametro sia un singolo carattere */
	if (strlen(argv[argc-1]) != 1)
	{
        	printf("Errore ultimo parametro %s non singolo carattere\n", argv[argc-1]);
        	exit(2);
	}

	/* individuiamo il carattere da cercare */
	Cx = argv[argc-1][0];
	/* controlliamo che sia un carattere alfabetico minuscolo */
	if (! islower(Cx)) /* N.B. per usare questa funzione bisogna includere string.h */
	{
        	printf("Errore ultimo parametro %c non alfabetico minuscolo\n", Cx);
        	exit(3);
	}

	N = argc-2;	/* calcoliamo il numero di file e quindi il numero di processi figli da creare */

	/* stampa di debugging */
	printf("DEBUG-Carattere da cercare '%c' e numero di processi da creare %d\n", Cx, N);

	/* allocazione dell'array di N pipe */
	if ((pipes=(pipe_t *)malloc(N * sizeof(pipe_t))) == NULL)
	{
        	printf("Errore allocazione pipe\n");
        	exit(4);
	}

	/* creazione delle N pipe, usate in pipeline */
	for (i=0; i < N; i++)
        	if(pipe(pipes[i])<0)
        	{
                	printf("Errore nella creazione della pipe di indice i = %d\n", i);
                	exit(5);
        	}

	/* creazione figli */
	for (i=0; i < N; i++)
	{
        	if ((pid=fork())<0)
        	{
                	printf("Errore creazione figlio %d-esimo\n", i);
                	exit(6);
        	}

        	if (pid==0)
        	{       /* codice figlio Pi */
                	printf("DEBUG-Sono il figlio di indice %d e pid %d associato al file %s\n", i, getpid(), argv[i+1]);
                	/* nel caso di errore in un figlio decidiamo di ritornare un valore via via crescente rispetto al massimo valore di i (cioe' rispetto N-1), supponendo comunque che sia minore o uguale di 255 */

                	/* chiusura dei lati delle pipe non usate nella pipeline */
			/* per capire le chiusure, fare riferimento al commento riportato per l'array pipes */
                	for (j=0; j < N; j++)
                	{
                        	if (j!=i)
                                	close (pipes[j][1]);
                        	if ((i == 0) || (j != i-1))
                                	close (pipes[j][0]);
                	}

                 	/* allocazione dell'array di strutture specifico di questo figlio */
                	/* allochiamo un array di dimensione i+1 anche se leggeremo i strutture, dato che poi ci servira' riempire la i+1-esima struttura! */
                	if ((cur=(s_occ *)malloc((i+1) * sizeof(s_occ))) == NULL)
                	{
                        	printf("Errore allocazione cur per il figlio di indice %d\n", i);
                        	exit(N);
                	}
                	/* inizializziamo l'ultima struttura (quella di posto i+1 che pero' ha chiaramente indice i) che e' quella specifica del figlio corrente (nel caso del primo figlio sara' l'unica struttura */
                	cur[i].Fpid = getpid();
                	cur[i].occ = 0L;

                	/* apertura file */
                	if ((fd=open(argv[i+1],O_RDONLY))<0)
                	{
                        	printf("Impossibile aprire il file %s\n", argv[i+1]);
                        	exit(N+1);
                	}

                        /* ogni figlio deve leggere il proprio file, un carattere alla volta */
			while (read(fd,&ch,1)>0)
                	{
                        	/* cerco il carattere */
                        	if (ch == Cx)
                        	{
                                	(cur[i].occ)++;
                                	/*
                                	printf("DEBUG-Sono il figlio di indice %d e pid %d e ho trovato %ld occorrenze del carattere %c\n", cur[i].id, getpid(), cur[i].occ, Cx);
                                	*/
                        	}
                	}

                	/* lettura da pipe dell'array di strutture per tutti i figli a parte il primo */
                	if (i!=0)
                	{
                        	nr=read(pipes[i-1][0],cur,i*sizeof(s_occ));
				/* N.B. In caso di pipeline e' particolarmente importante che la lettura sia andata a buon fine e che quindi la pipeline non si sia 'rotta'; si deve quindi controllare il valore di ritorno della read da pipe! */
                        	if (nr != i*sizeof(s_occ))
                        	{
                                	printf("Figlio %d ha letto un numero di strutture sbagliate %d\n", i, nr);
                                	exit(N+2);
                        	}
                        	/*
                        	for(j=0;j<i;j++)
                                	printf("DEBUG-Ho ricevuto da figlio di indice %d trovato %ld occorrenze del carattere %c\n", cur[j].id, cur[j].occ, Cx);
                        	*/
                	}

                	/* tutti i figli mandano in avanti (cioe' al figlio successivo, a parte l'ultimo figlio che manda al padre) un array di strutture (per tutti i figli, a parte il primo, sono 'i' strutture ricevute dal processo precedente e la 'i+1-esima' che e' la propria); nel caso del primo figlio l'array mandato avanti contiene un solo elemento costituito ddalla propria struttura */
                	/*
                	printf("DEBUG-Sto per mandare al figlio seguente %d strutture che sono: \n", i+1);
                	for(j=0;j<i+1;j++)
                        	printf("DEBUG-Sto per mandare processo %d trovato %ld occorrenze del carattere %c\n", cur[j].id, cur[j].occ, Cx);
                	*/
                	nw=write(pipes[i][1],cur,(i+1)*sizeof(s_occ));
			/* N.B. In caso di pipeline e' particolarmente importante che anche la scrittura sia andata a buon fine e che quindi la pipeline non si sia 'rotta'; si deve quindi controllare il valore di ritorno della write su pipe! */
			if (nw != (i+1)*sizeof(s_occ))
			{
				printf("Figlio %d ha scritto un numero di byte sbagliati %d\n", i, nw);
				exit(N+3);
			}

                	/* ogni figlio deve ritornare al padre il proprio indice d'ordine */
			exit(i); 
        	}
	} /* fine for */

	/* codice del padre */
	/* chiusura lati delle pipe non usati: tutti meno quello l'ultima in lettura */
	for (i=0; i < N; i++)
	{
        	close (pipes[i][1]);
        	if (i != N-1) close (pipes[i][0]);
	}

	/* allocazione dell'array di strutture specifico per il padre */
	/* allochiamo un array di dimensione N quanto il numero di figli! */
	if ((cur=(s_occ *)malloc(N * sizeof(s_occ))) == NULL)
	{
        	printf("Errore allocazione cur nel padre\n");
        	exit(7);
	}

	/* il padre deve leggere l'array di strutture che gli arriva dall'ultimo figlio */
	/* quindi al padre arriva una singola informazione, rappresentata pero' da un array che quindi contiene piu' elementi */
	nr=read(pipes[N-1][0],cur,N*sizeof(s_occ));
	if (nr != N*sizeof(s_occ))
	{
        	printf("Padre ha letto un numero di strutture sbagliate %d\n", nr);
        	/* deciso di commentare questa
		exit(8);
		cosi' se i figli incorrono in un qualche problema e quindi la pipeline si rompe, il padre comunque eseguira' l'attesa dei figli */
	}
	else
	{
		nr=nr/sizeof(s_occ);
		printf("DEBUG-Padre ha letto un numero di strutture %d\n", nr);
		/* il padre deve stampare i campi delle strutture ricevute insieme con tutte le altre informazioni richieste dal testo (carattere e nome file) */
		for(i=0; i < N; i++)
        	printf("Il figlio di indice %d e pid %d ha trovato %ld occorrenze del carattere %c nel file %s\n", i, cur[i].Fpid, cur[i].occ, Cx, argv[i+1]);
	}

	/* Il padre aspetta i figli */
	for (i=0; i < N; i++)
	{
        	pidFiglio = wait(&status);
        	if (pidFiglio < 0)
        	{
                	printf("Errore in wait\n");
                	exit(9);
        	}

        	if ((status & 0xFF) != 0)
                	printf("Figlio con pid %d terminato in modo anomalo\n", pidFiglio);
        	else
        	{
                	ritorno=(int)((status >> 8) & 0xFF);
                	printf("Il figlio con pid=%d ha ritornato %d (se > di %d problemi)\n", pidFiglio, ritorno, N-1);
        	}
	}

	exit(0);
}