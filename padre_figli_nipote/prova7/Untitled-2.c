/* Soluzione della Prova d'esame del 12 Settembre 2018 - Parte C */
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <ctype.h>

/* definizione del TIPO pipe_t come array di 2 interi */
typedef int pipe_t[2];

int main(int argc, char **argv) 
{
   /* -------- Variabili locali ---------- */
   int pid;			/* process identifier per le fork() */
   int N; 			/* numero di file passati sulla riga di comando e quindi di figli da creare */
   pipe_t *pipedFP;		/* array dinamico di pipe per comunicazioni figli-padre  */
   pipe_t *pipedNP;		/* array dinamico di pipe per comunicazioni nipoti-padre  */
   int fd;			/* file descriptor che serve sia ai figli che ai nipoti */
   int i, j;			/* indici per i cicli */
   char ch; 			/* variabile che serve per leggere i caratteri sia da parte dei figli che dei nipoti */
   long int trasformazioni=0L;	/* variabile (inizializzata a 0) che mantiene il numero delle trasformazioni effettuate sia dai figli che dai nipoti */
   int nr;			/* variabile per salvare valori di ritorno di read da pipe */ 
   int ritorno; 		/* variabile che viene ritornata da ogni figlio al padre e da ogni nipote al figlio */
   int status;			/* variabile di stato per la wait */
   /* ------------------------------------ */
	
	/* controllo sul numero di parametri: almeno di due nomi di file */
	if (argc < 3) 
	{
		printf("Errore: Necessari per %s almeno 2 parametri (almeno due nomi di file) e invece argc = %d\n", argv[0], argc);
		exit(1);
	}

	/* calcoliamo il numero di file passati e quindi di figli da creare */
	N = argc - 1;
	
	/* allocazione dell'array di N pipe figli-padre */
	pipedFP = (pipe_t *) malloc(N*sizeof(pipe_t));
	if (pipedFP == NULL)
	{
		printf("Errore nella allocazione della memoria pipe figli-padre\n");
		exit(2);
	}
	
	/* allocazione dell'array di N pipe nipoti-padre */
	/* N.B. In questo caso i nipoti devono comunicare direttamente con il padre e quindi e' il padre che deve creare una pipe per ogni nipote e non il figlio come invece si DEVE fare nei testi in cui la comunicazione deve essere nipote-figlio */
	pipedNP = (pipe_t *) malloc (N*sizeof(pipe_t));
	if (pipedNP == NULL)
	{
		printf("Errore nella allocazione della memoria pipe nipoti-padre\n");
		exit(3);
	}
	
	/* creazione delle N pipe figli-padre e delle N pipe nipoti-padre */
	for (i=0; i < N; i++)
	{
		if (pipe(pipedFP[i]) < 0)
		{
			printf("Errore nella creazione della pipe %d-esima figli-padre\n", i);
			exit(4);
		}
		if (pipe(pipedNP[i]) < 0)
		{
			printf("Errore nella creazione della pipe %d-esima nipoti-padre\n", i);
			exit(5);
		}
	}

	/* ciclo di generazione dei figli */
	for (i=0; i < N; i++)
	{
		if ( (pid = fork()) < 0)
		{
			printf("Errore nella fork %d-esima\n", i);
			exit(6);
		}
		
		if (pid == 0) 
		{
			/* codice del figlio Pi */
			printf("DEBUG-Sono il processo figlio di indice %d e pid %d sto per creare il nipote che leggera' sempre dal mio stesso file %s\n", i, getpid(), argv[i+1]);

			/* chiusura delle pipe non usate nella comunicazione con il padre  */
			for (j=0; j < N; j++)
			{
				close(pipedFP[j][0]);
				if (i != j) close(pipedFP[j][1]);
			}

			/* N.B. Le chiusure delle altre pipe vanno fatte assolutamente dopo la creazione del nipote! */
			if ( (pid = fork()) < 0)
			{
				printf("Errore nella fork %d-esima di creazione del nipote\n", i);
				exit(-1); /* decidiamo, in caso di errore, di tornare -1 che verra' interpretato come 255 e quindi un valore NON accettabile */
			}	

			if (pid == 0) 
			{
				/* codice del nipote PPi */
				printf("DEBUG-Sono il processo nipote del figlio di indice %d e pid %d\n", i, getpid());

				/* chiusura della pipe rimasta aperta di comunicazione fra figlio-padre che il nipote non usa */
				close(pipedFP[i][1]);
				/* chiusura delle pipe non usate nella comunicazione con il padre  */
				for (j=0; j < N; j++)
				{
					close(pipedNP[j][0]);
					if (i != j) close(pipedNP[j][1]);
				}

				/* sia il figlio che il nipote devono aprire (con due open separate per avere l'I/O pointer separato) lo stesso file in lettura/scrittura, dato che dovranno operare delle trasformazioni */
				if ((fd=open(argv[i+1], O_RDWR)) < 0)
				{
                                	printf("Errore nella open del file %s da parte del nipote\n", argv[i+1]);
					exit(-1); /* decidiamo, in caso di errore, di tornare -1 che verra' interpretato come 255 e quindi un valore NON accettabile */
                        	}

				/* si deve leggere dal file un carattere alla volta */
		        	while (read(fd, &ch, 1))
				{	
					/* siamo nel NIPOTE e quindi controlliamo se abbiamo trovato un carattere alfabetico minuscolo */
					if (islower(ch))  
					{ 	
						/* questo carattere deve essere trasformato nel corrispondente carattere alfabetico MAIUSCOLO */ 
					  	ch = ch - 32; /* trasformiamo il carattere da minuscolo in MAIUSCOLO togliendo 32: volendo si poteva usare anche la funzione di libreria toupper! */
						/* prima di scriverlo bisogna tornare indietro di una posizione */
						lseek(fd, -1L, SEEK_CUR);
					  	write(fd, &ch, 1); /* scriviamolo sul file */
						/* e aggiorniamo il numero di trasformazioni */
						trasformazioni++;
					}
					/* altrimenti NON si deve fare nulla */
				}

				/* il nipote deve inviare al padre il numero di trasformazioni effettuate */
				write(pipedNP[i][1], &trasformazioni, sizeof(trasformazioni));

				/* torniamo il valore richiesto dal testo eseguendo una divisione intera per 256 */
				ritorno=trasformazioni/256;	
				exit(ritorno);
			}

			/* codice figlio */
			/* le pipe usate dal nipote vanno chiuse TUTTE */
			/* ATTENZIONE: SOLO DOPO AVERE CREATO IL NIPOTE! */
			for (j=0; j < N; j++)
			{
				close(pipedNP[j][0]);
				close(pipedNP[j][1]);
			}

			/* il figlio ha un codice molto simile al nipote */
			/* sia il figlio che il nipote devono aprire (con due open separate per avere l'I/O pointer separato) lo stesso file in lettura/scrittura, dato che dovranno operare delle trasformazioni */
			if ((fd=open(argv[i+1], O_RDWR)) < 0)
			{
                               	printf("Errore nella open del file %s da parte del figlio\n", argv[i+1]);
				exit(-1); /* decidiamo, in caso di errore, di tornare -1 che verra' interpretato come 255 e quindi un valore NON accettabile */
                        }
		       	
			/* si deve leggere dal file un carattere alla volta */
			while (read(fd, &ch, 1))
			{	
				/* controlliamo se abbiamo trovato un carattere numerico */
				if (isdigit(ch))  
				{ 	
					/* questo carattere deve essere trasformato nel carattere spazio */
				  	ch = ' '; /* trasformiamo il carattere nello spazio */
					/* prima di scrErrore nella lettura dal figlio di indice 2
Errore nella open del file F da parte del nipote
Errore nella lettura dal nipote di indice 2
iverlo bisogna tornare indietro di una posizione */
					lseek(fd, -1L, SEEK_CUR);
				  	write(fd, &ch, 1); /* scriviamolo sul file */
					/* e aggiorniamo il numero di trasformazioni */
					trasformazioni++;
				}
				/* altrimenti NON si deve fare nulla */
			}

			/* il figlio deve inviare al padre il numero di trasformazioni operate */
			write(pipedFP[i][1], &trasformazioni, sizeof(trasformazioni));

			/* il figlio deve aspettare il nipote e stampare il suo pid con il valore ritornato (come richiesto dal testo) */
			if ((pid = wait(&status)) < 0)
			{	
				printf("Errore in wait\n");
				/* dato che il figlio deve tornare al padre un proprio valore segnaliamo solo il problema */
			}
			if ((status & 0xFF) != 0)
			{	
    				printf("Nipote con pid %d terminato in modo anomalo\n", pid);
				/* dato che il figlio deve tornare al padre un proprio valore segnaliamo solo il problema */
			}
    			else
				printf("Il nipote con pid=%d ha ritornato %d\n", pid, ritorno=(int)((status >> 8) & 0xFF));

			/* torniamo il valore richiesto dal testo operando una divisione intera per 256 */
			ritorno=trasformazioni/256;	
			exit(ritorno); 
		}
	}
	
	/* codice del padre */
	/* il padre chiude i lati delle pipe che non usa, cioe' tutti quelli di scrittura */
	for (i=0; i < N; i++)
	{
		close(pipedFP[i][1]);
		close(pipedNP[i][1]);
 	}

	/* Il padre recupera le informazioni dai figli e dai nipoti in ordine di indice */
	for (i=0; i < N; i++)
	{
		nr=read(pipedFP[i][0], &trasformazioni, sizeof(trasformazioni));
		if (nr == sizeof(trasformazioni))
			printf("Il figlio di indice %d ha operato %ld trasformazioni di caratteri numerici in carattere spazio sul file %s\n", i, trasformazioni, argv[i+1]);
		else
			printf("Errore nella lettura dal figlio di indice %d\n", i);

		nr=read(pipedNP[i][0], &trasformazioni, sizeof(trasformazioni));
		if (nr == sizeof(trasformazioni))
			 printf("Il nipote di indice %d ha operato %ld trasformazioni di caratteri minuscoli in MAIUSCOLI sullo stesso file %s\n", i, trasformazioni, argv[i+1]);
		else
		        printf("Errore nella lettura dal nipote di indice %d\n", i);

	}	

	/* Il padre aspetta i figli */
	for (i=0; i < N; i++)
	{
		if ((pid = wait(&status)) < 0)
		{
			printf("Errore in wait\n");
			exit(7);
		}

		if ((status & 0xFF) != 0)
    			printf("Figlio con pid %d terminato in modo anomalo\n", pid);
    		else
		{ 
			ritorno=(int)((status >> 8) &	0xFF); 
		  	if (ritorno==255)
 				printf("Il figlio con pid=%d ha ritornato %d e quindi vuole dire che ci sono stati dei problemi\n", pid, ritorno);
		  	else  	printf("Il figlio con pid=%d ha ritornato %d\n", pid, ritorno);
		}
	}

	exit(0);
}