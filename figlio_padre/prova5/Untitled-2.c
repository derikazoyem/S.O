/* Soluzione della Prova d'esame dell'11 Luglio 2018 - Parte C */
/* Versione in cui il padre calcola il massimo */
#include <stdio.h> 
#include <unistd.h> 
#include <stdlib.h> 
#include <sys/wait.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/stat.h> 
#include <fcntl.h> 

/* definizione del TIPO pipe_t come array di 2 interi */
typedef int pipe_t[2]; 		

/* VARIABILI GLOBALI dato che servono sia nel main che nella funzione finitof! */
int *finito; 	/* array dinamico per indicare i figli che sono terminati */
		/* la semantica di questo array e' che ogni elemento vale 0 se il corrispondente processo figlio NON e' finito, altrimenti vale 1 */
		/* OSSERVAZIONE: in questo caso NON e' sufficiente una singola variabile finito: si veda nota nel codice del padre */
int N; 		/* numero di processi figli */

int finitof()
{
	/* questa funzione verifica i valori memorizzati nell'array finito: appena trova un elemento uguale a 0 vuole dire che non tutti i processi figli sono finiti e quindi torna 0; tornera' 1 se e solo se tutti gli elementi dell'array sono a 1 e quindi tutti i processi figli sono finiti */
   int i;
	for (i=0; i < N; i++)
		if (!finito[i])
			/* appena ne trova uno che non ha finito */
			return 0; /* ritorna falso */
	return 1;
}

int main(int argc, char **argv) 
{
   int pid;				/* variabile per fork */
   pipe_t *pipe_fp;			/* array di pipe per la comunicazione figli-padre*/
   pipe_t *pipe_pf;			/* array di pipe per la sincronizzazione padre-figli */
   char CZ; 				/* carattere da cercare */
   int fd;				/* variabile per open */
   char ch;				/* variabile per leggere dai file */
   char chControllo;			/* variabile per inviare indicazione ai figli */
   long int posizione; 			/* QUESTA VARIABILE TIENE TRACCIA DELLA POSIZIONE NEL FILE, SENZA USARE LSEEK. La posizione del primo carattere e' assunta uguale a 0: attenzione che se si usasse la lseek (dopo l'individuazione del carattere cercato) il calcolo della posizione partirebbe da 1 dato che dopo la read il file pointer risulta gia' incrementato e quindi se si volesse mantenere la stessa semantica usata in questa soluzione bisognerebbe devrementare il valore tornato dalla lseek! */
   long int posizioneMax; 		/* usata dal padre per calcolare il massimo */
   int indice; 				/* usata dal padre per tenere traccia dell'indice del figlio che ha calcolato il massimo */
   int occorrenze;			/* numero di occorrenze trovate che va ritornato al padre */
   int i, j;				/* indici per cicli */
   int nr, nw;				/* per controlli read e write su/da pipe */
   int status, pidFiglio, ritorno;	/* per wait */

   	/* controllo sul numero di parametri: un carattere e almeno due nomi di un file */
	if (argc < 4) 
	{
		printf("Errore: Necessari per %s almeno 3 parametri (un carattere e almeno due nomi di file) e invece argc = %d\n", argv[0], argc);
		exit(1);
	}

	/* controlliamo se il primo parametro e' un singolo carattere */
	if (strlen(argv[1]) != 1)
	{
		printf("Errore primo parametro non singolo carattere %s\n", argv[1]);
		exit(2);
	}

	CZ = argv[1][0]; /* isoliamo il carattere che devono cercare i figli */

	/* calcoliamo il numero di file passati e quindi di figli da creare */
	N = argc - 2; /* ogni figlio e' associato a argv[2], argv[3]...argv[argc-1] */
        
	printf("DEBUG-Carattere da cercare '%c' e numero di processi da creare %d\n", CZ, N);

	/* allocazione memoria dinamica per l'array finito */
	finito = (int *) malloc(sizeof(int) * N);
	if(finito == NULL)
	{
		printf("Errore nella allocazione della memoria per array finito\n");
		exit(3);
	}
	
	/* ... e inizializzazione a 0: all'inizio nessun figlio e' finito */
	memset(finito, 0, N*sizeof(int));

	/* allocazione memoria dinamica per pipe_fp e pipe_pf */
	pipe_fp=malloc(N*sizeof(pipe_t));
	pipe_pf=malloc(N*sizeof(pipe_t));
	if ((pipe_fp == NULL) || (pipe_pf == NULL))
	{
		printf("Errore nelle malloc\n");
		exit(4);
	}

	/* creazione delle N + N pipe */
	for (i=0; i < N; i++) 
	{
		if (pipe(pipe_fp[i])!=0) 
		{
			printf("Errore creazione della pipe figli-padre di indice %d\n", i);
			exit(5);
		}
		if (pipe(pipe_pf[i])!=0) 
		{
			printf("Errore creazione della pipe padre-figli di indice %d\n", i);
			exit(6);
		}
	}

	/* ciclo di creazione dei processi figli */
	for (i=0; i < N; i++) 
	{
		if ((pid=fork()) < 0)  /* errore */
   		{
			printf("Errore nella fork con indice %d\n", i);
      			exit(7);
   		}

		if (pid == 0) 
		{
 			/* codice del figlio Pi */
			/* stampa di debugging */
      			printf("DEBUG-Figlio di indice %d e pid %d associato al file %s\n",i,getpid(), argv[i+2]);

      			/* chiudiamo le pipe che non servono */
      			/* ogni figlio scrive solo su pipe_fp[i] e legge solo da pipe_pf[i] */
      			for (j=0; j < N; j++)
      			{
        			close(pipe_fp[j][0]);
        			close(pipe_pf[j][1]);
        			if (j!=i)
        			{
        				close(pipe_fp[j][1]);
        				close(pipe_pf[j][0]);
        			}
      			}

			/* apriamo il file: ATTENZIONE ALL'INDICE! */
			if ((fd=open(argv[i+2], O_RDONLY)) < 0) 
			{
				printf("Errore: Impossibile aprire il file %s\n", argv[i+2]);
				exit(-1); /* in caso di errore torniamo -1 che verra' interpretato dal padre come 255 che non e' un valore accettabile */
			}

			/* inizializziamo sia posizione che occorrenze */
			posizione=0L; 	
			occorrenze=0;

			/* si deve leggere dal file un carattere alla volta */
	 		while (read(fd, &ch, 1)) 	
			{
				if (ch==CZ) /* se troviamo una occorrenza del carattere cercato */
				{
					occorrenze++; /* incrementiamo il numero di occorrenze */
					/* e inviamo la posizione corrente al padre */
					nw=write(pipe_fp[i][1], &posizione, sizeof(posizione));
					if (nw != sizeof(posizione))
    					{			
                        			printf("Errore: Impossibile scrivere sulla pipe per il processo di indice %d\n", i);
                        			exit(-1); 
               	 			}
					/* aspettiamo dal padre l'indicazione se dobbiamo scrivere o meno */
					nr=read(pipe_pf[i][0], &chControllo, 1);
					if (nr != 1)
    					{		
                        			printf("Errore: Impossibile leggere dalla pipe per il processo di indice %d\n", i);
                        			exit(-1); 
         		 		}		
					if (chControllo =='S') 
					{
						/* il padre ha autorizzato, scriviamo */
						printf("Sono il figlio di indice %d e pid %d e ho trovato una occorrenza del carattere '%c' alla posizione %ld nel file %s\n", i, getpid(), CZ, posizione,  argv[i+2]);
					}	 
					/* l'else e' INUTILE, dato che in caso il carattere ricevuto si uguale a 'N' si deve continuare a leggere */
				}
				posizione++; 	/* incrementiamo sempre il valore di posizione per il prossimo carattere */
			}	

			exit(occorrenze); /* torniamo il numero di occorrenze trovate (supposto dal testo < di 255) */
		}
	}

	/*codice del padre*/
	/* chiudiamo le pipe che non usiamo: il padre legge da tutte le pipe pipe_fp e scrive su tutte le pipe pipe_pf */
	for (i=0; i < N; i++)
	{
   		close(pipe_fp[i][1]);
   		close(pipe_pf[i][0]);
	}

	/* continuiamo a ciclare su tutti i figli in ordine, finche' tutti hanno finito */
	while (!finitof())
	{
		posizioneMax=-1L; /* ATTENZIONE: il valore del massimo, va resettato per ogni insieme di valori letti dal padre */
		for (i=0; i < N; i++) 
		{
			/* tentiamo di leggere la posizione dal figlio i-esimo: contestualmente viene aggiornato il valore del corrispondente elemento dell'array finito */
			finito[i] = (read(pipe_fp[i][0], &posizione, sizeof(posizione)) != sizeof(posizione));  /* nel caso la read torni un valore diverso da sizeof(posizione), il corrispondente valore di finito viene settato a 1 dato che la condizione e' vera e quindi viene tenuta traccia che il corrispondete figlio e' finito! */
			if (!finito[i]) /* se invece il figlio corrente NON e' finito */
			{
				if (posizione > posizioneMax)
				{
					/* dobbiamo aggiornare il massimo */
					posizioneMax = posizione;
					/* debbiamo tenere conto dell'indice del processo che ha inviato il massimo */
					indice = i;
				}
			}
		}	

		/* bisogna mandare ai figli l'indicazione giusta (S per scrivere, N per NON scrivere) */
		/* OSSERVAZIONE IMPORTANTE: IN QUESTA SOLUZIONE NON BASTA LA VARIABILE FINITO DATO CHE LA FASE DI SCRITTURA DEL PADRE AI FIGLI VIENE FATTA NEL CICLO FOR CHE SEGUE E QUINDI IL PADRE DEVE AVERE MODO DI FARE LA SCRITTURA SE E SOLO SE IL FIGLIO GLI HA INVIATO QUALCOSA E QUINDI NON E' FINITO, ALTRIMENTI (IN CASO DI ASSENZA DI QUESTO CONTROLLO) SE UN FIGLIO E' FINITO E IL PADRE GLI MANDA COMUNQUE IL CARATTERE DI CONTROLLO, IL PADRE RICEVEREBBE UN SEGNALE SIGPIPE E MORIREBBE (A MENO CHIARAMENTE DI NON GESTIRE QUESTO SEGNALE CON UNA FUNZIONE HANDLER SPECIFICA)! */ 
		for (i=0; i < N; i++) 
		{
			if (i == indice) /* figlio che deve scrivere */
				chControllo = 'S';
			else 	chControllo = 'N';
			if (!finito[i])	/* mandiamo il carattere di controllo SOLO ai figli NON finiti */
			{
				nw=write(pipe_pf[i][1], &chControllo, 1); 
				if (nw != 1)
					printf("Errore: PADRE non e' riuscito ad inviare nulla al figlio di indice %d \n", i);
			}	
		}
	}

	/* Attesa della terminazione dei figli */
	for (i=0; i < N; i++)
	{
   		if ((pidFiglio = wait(&status)) < 0)
   		{
      			printf("Errore wait\n");
      			exit(8);
   		}
   		if ((status & 0xFF) != 0)
                	printf("Figlio con pid %d terminato in modo anomalo\n", pidFiglio);
   		else
   		{
                	ritorno=(int)((status >> 8) & 0xFF);
                	printf("Il figlio con pid=%d ha ritornato %d (se 255 errore)\n", pidFiglio, ritorno);
   		}
	}	

	exit(0);
}/* fine del main */