/* Soluzione della parte C del compito del 7 Settembre 2022 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h> 
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#define PERM 0644

typedef int pipe_t[2];

int main (int argc, char **argv)
{
   int N; 				/* numero di file/processi */
   /* ATTENZIONE NOME N imposto dal testo! */
   int pid;             		/* pid per fork */
   pipe_t *pipes;  			/* array di pipe usate a pipeline da primo figlio, a secondo figlio .... ultimo figlio e poi a padre: ogni processo (a parte il primo) legge dalla pipe i-1 e scrive sulla pipe i */
   int n,j; 				/* indici */
   /* ATTENZIONE NOME n imposto dal testo! */
   int fd, fcreato; 			/* file descriptor */
   /* ATTENZIONE NOME fcreato imposto dal testo! */
   int pidFiglio, status, ritorno;	/* per valore di ritorno figli */
   int nroCar;				/* variabile per salvare la lunghezza in caratteri dei file e quindi capire se si e' su un carattere di posizione dispari */
   char car;				/* variabile per salvare i caratteri letti da ogni file */
   /* ATTENZIONE car imposto dal testo! */
   char *cur;        			/* array dinamico di caratteri */
   /* ATTENZIONE NOME cur imposto dal testo! */
   int nr,nw;              		/* variabili per salvare valori di ritorno di read/write da/su pipe */

	/* controllo sul numero di parametri: almeno 2 file */
	if (argc < 3)
	{
		printf("Errore numero di parametri, argc=%d\n", argc);
		exit(1);
	}

	N = argc-1;
	printf("DEBUG-Numero di processi da creare %d\n", N);

	/* allocazione pipe */
	if ((pipes=(pipe_t *)malloc(N*sizeof(pipe_t))) == NULL)
	{
		printf("Errore allocazione pipe\n");
		exit(2); 
	}

	/* creazione pipe */
	for (n=0;n<N;n++)
		if(pipe(pipes[n])<0)
		{
			printf("Errore creazione pipe\n");
			exit(3);
		}

	/* allocazione array di caratteri: la puo' fare il padre per tutti i figli */
	if ((cur=(char *)malloc(N*sizeof(char))) == NULL)
	{
		printf("Errore allocazione array di caratteri\n");
		exit(4); 
	}

	/* creiamo il file nella directory corrente avente come nome il mio cognome (TUTTO IN MAIUSCOLO, come specificato nel testo) */
	if ((fcreato=creat("LEONARDI", PERM)) < 0)
	{
        	printf("Errore nella creat del file %s\n", "Leonardi");
        	exit(5);
	}

	/* creazione figli */
	for (n=0;n<N;n++)
	{
		if ((pid=fork())<0)
		{
			printf("Errore creazione figlio\n");
			exit(6);
		}
		if (pid == 0)
		{ 	/* codice figlio */
			printf("DEBUG-Sono il figlio %d e sono associato al file %s\n", getpid(), argv[n+1]);
			/* nel caso di errore in un figlio decidiamo di ritornare il valore -1 che sara' interpretato dal padre come 255 (valore NON ammissibile) */

			/* chiusura pipes inutilizzate */
			for (j=0;j<N;j++)
			{	/* si veda commento nella definizione dell'array pipes per comprendere le chiusure */
                		if (j!=n)
                			close (pipes[j][1]);
                		if ((n == 0) || (j != n-1))
               				close (pipes[j][0]);
        		}
	
			/* apertura file */
			if ((fd=open(argv[n+1],O_RDONLY))<0)
			{
				printf("Impossibile aprire il file %s\n", argv[n+1]);
				exit(-1);
			}
	
			/* inizializziamo il numero dei caratteri letti */
			nroCar = 0;
			/* con un ciclo leggiamo tutti caratteri, come richiede la specifica */
        		while(read(fd,&car,1) != 0)
        		{
				nroCar++;	/* incrementiamo il numero dei caratteri letti */
          			if ((nroCar % 2) == 1) /* siamo su un carattere di posizione dispari */
                		{
					if (n != 0)
	                		{
						/* se non siamo il primo figlio, dobbiamo aspettare l'array dal figlio precedente per mandare avanti */
                				nr=read(pipes[n-1][0],cur,N*sizeof(char));
        					/* per sicurezza controlliamo il risultato della lettura da pipe */
                				if (nr != N*sizeof(char))
                				{
                       					printf("Figlio %d ha letto un numero di byte sbagliati %d\n", n, nr);
                       					exit(-1);
                				}
                			}	
					/* a questo punto si deve inserire il carattere letto nel posto giusto */
					cur[n]=car;
				
					/* ora si deve mandare l'array in avanti */
        				nw=write(pipes[n][1],cur,N*sizeof(char));
        				/* anche in questo caso controlliamo il risultato della scrittura */
        				if (nw != N*sizeof(char))
        				{
               					printf("Figlio %d ha scritto un numero di byte sbagliati %d\n", n, nw);
               					exit(-1);
        				}
					ritorno=car; /* salviamo il carattere dispari perche' potrebbe essere quello da ritornare! IN REALTA' CONSIDERANDO LA SPECIFICA DELLA PARTE SHELL E CIOE' CHE LA LUNGHEZZA DEI FILE E' DISPARI QUESTO SALVATAGGIO NON SERVIREBBE E BASTEREBBE TORNARE car CHE E' SICURAMENTE, COME ULTIMO CARATTERE, IN POSIZIONE DISPARI! */
        			}	
				else
				; /* NON SI DEVE FARE NULLA */
        		}		
			/* ogni figlio deve tornare l'ultimo carattere letto */
			exit(ritorno);
		}
	} /* fine for */

	/* codice del padre */
	/* chiusura di tutte le pipe che non usa */
	for (n=0;n<N;n++) 
	{
		close (pipes[n][1]);
		if (n != N-1) close (pipes[n][0]); 
	}

	/* il padre deve leggere tutti gli array di caratteri inviati dall'ultimo figlio */
	while (read(pipes[n-1][0],cur,N*sizeof(char)))
	{
		/* il padre deve scrivere i caratteri sul file creato: si puo' usare una singola write di N caratteri! */
		write(fcreato, cur, N*sizeof(char));
	}	

	/* Il padre aspetta i figli */
	for (n=0; n < N; n++)
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
        		printf("Il figlio con pid=%d ha ritornato %c (e in decimale %d se 255 problemi)\n", pidFiglio, ritorno, ritorno);
        	} 
	}
	exit(0);
}