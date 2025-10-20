/* Soluzione della parte C del compito del 6 Settembre 2023 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h> 
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>

typedef int pipe_t[2];
typedef struct{
	long int tot; 		/* numero minimo di linee (campo c1 del testo) */
        int id; 		/* indice figlio (campo c2 del testo) */
		} s_tot;

int main (int argc, char **argv)
{
  int N; 			/* numero di file */
  int *pid;		/* array di pid per fork */
  pipe_t *pipes;		/* array di pipe usate a pipeline da primo figlio, a secondo figlio .... ultimo figlio e poi a padre: ogni processo (a parte il primo) legge dalla pipe i-1 e scrive sulla pipe i */
  pipe_t *pipe_pf;        /* array di pipe per la comunicazione padre-figli */
  int n,j; 		/* indici */
  int fd; 		/* file descriptor */
  int pidFiglio, status, ritorno;	/* per valore di ritorno figli */
  long int cur_tot; 	/* conteggio delle linee calcolate da ogni figlio */
  long int min; 		/* variabile per ricevere il valore inviato dal padre */
  s_tot letta; 		/* struttura usata dai figli e dal padre */
  char linea[250];    	/* per leggere le linee dal file, dato che il testo specifica che non sia piu' lunga di 250 compreso il terminatore di linea (che verra' poi trasformato nel terminatore di stringa per poterla stampare con printf */
int nr,nw;              /* variabili per salvare valori di ritorno di read/write da/su pipe */

/* controllo sul numero di parametri almeno 2 file */
if (argc < 3)
{
	printf("Errore numero di parametri dato che argc=%d\n", argc);
	exit(1);
}

N = argc-1;
printf("DEBUG-Numero di processi da creare %d\n", N);

/* allocazione pid: necessaria perche' il padre deve fare una stampa del pid del processo che ha calcolaro il minimo */
if ((pid=(int *)malloc(N*sizeof(int))) == NULL)
{
	printf("Errore allocazione pid\n");
	exit(2); 
}

/* allocazione dei 2 array di pipe */
pipes=(pipe_t *)malloc(N*sizeof(pipe_t)); 	/* pipe usate in pipeline */
pipe_pf=malloc(N*sizeof(pipe_t));		/* pipe da padre ai figli */

if ((pipes == NULL) || (pipe_pf == NULL))
{
	printf("Errore nelle malloc\n");
        exit(3);
}

/* creazione delle pipe */
for (n=0; n < N; n++)
{
	if (pipe(pipes[n])!=0)
        {
                printf("Errore creazione delle pipe figli-padre\n");
                exit(4);
        }
        if (pipe(pipe_pf[n])!=0)
        {
                printf("Errore creazione delle pipe padre-figli\n");
                exit(5);
        }
}

/* creazione figli */
for (n=0;n < N; n++)
{
	if ((pid[n]=fork())<0)
	{
		printf("Errore creazione figlio\n");
		exit(5);
	}
	if (pid[n]==0)
	{ /* codice figlio */
	printf("DEBUG-Sono il figlio di indice %d e pid %d e sono associato al file %s\n", n, getpid(), argv[n+1]);
	/* nel caso di errore in un figlio decidiamo di ritornare un valore via via crescente rispetto al massimo valore di n (che e' N-1) */
	/* chiusura pipes inutilizzate */
	for (j=0; j < N; j++)
	{
		close(pipe_pf[j][1]);	/* il figlio NON scrive sulla pipe padre-figli, ma legge solo */
		if (j!=n)
		{
                        close(pipe_pf[j][0]);
			close(pipes[j][1]);
		}
		if ((n == 0) || (j != n-1))
			close(pipes[j][0]);
	}
 
	/* apertura file */
	if ((fd=open(argv[n+1],O_RDONLY))<0)
	{
		printf("Impossibile aprire il file %s\n", argv[n+1]);
		exit(N);
	}
	
	/* inizializziamo il contatore di linee */
	cur_tot = 0L;
	/* inizializziamo l'indice del buffer linea */
	j=0;
	/* con un ciclo leggiamo tutte le linee e calcoliamo quindi quante sono */
        while (read(fd, &(linea[j]), 1) != 0)         /* leggiamo un carattere alla volta e lo inseriamo nell'array linea, anche se in questo caso il salvataggio in un buffer non servirebbe, ma in questo modo il codice e' simile a quello che dovremo scrivere dopo! */
        {
                if (linea[j] == '\n')  /* se il carattere corrente e' il terminatore di linea ... */
                {
			cur_tot++; /* incrementiamo il numero di linee */
                        j = 0;     /* azzeriamo l'indice per la prossima linea */
                }
                else j++;          /* altrimenti incrementiamo l'indice di linea */
        }

	if (n == 0)
	{  /* il figlio di indice 0 deve preparare la struttura da mandare al figlio seguente */
		letta.id = 0;
		letta.tot = cur_tot;
    	}
	else
 	{ /* tutti gli altri figli devono leggere dal figlio precedente una struttura */	
		nr=read(pipes[n-1][0],&letta,sizeof(s_tot));
		/* per sicurezza controlliamo il risultato della lettura da pipe */
		if (nr != sizeof(s_tot))
        	{	
        		printf("Figlio %d ha letto un numero di byte sbagliati %d\n", n, nr);
        		exit(N+1);
        	}
 /*
		printf("HO ricevuto da figlio di indice %d che ha calcolato %ld linee dal file %s\n", letta.id, letta.tot,  argv[i+1]);
  */
		if (letta.tot > cur_tot)
	        {  /* il figlio di indice i ha calcolato un numero di linee minore e quindi bisogna aggiornare i valori di letta */
        		letta.id = n;
        		letta.tot = cur_tot;
        	}	
	}

	/* tutti i figli mandano in avanti, l'ultimo figlio manda al padre una struttura */
	nw=write(pipes[n][1],&letta,sizeof(s_tot));
	/* anche in questo caso controlliamo il risultato della scrittura */
	if (nw != sizeof(s_tot))
        {
              	printf("Figlio %d ha scritto un numero di byte sbagliati %d\n", n, nw);
                exit(N+2);
	}

	/* a questo punto i figli devono attendere il calcolo del minimo che gli deve inviare il padre, ricordandosi di ritornare all'inizio del file */
	lseek(fd, 0L, 0);
	read(pipe_pf[n][0], &min, sizeof(min));
	/* inizializziamo l'indice del buffer linea */
	j=0;
	/* inizializziamo il contatore di linee */
	cur_tot = 0L;
	/* con un ciclo, analogo a prima, ri-leggiamo tutte le linee e ci fermeremo su quella indicata dal padre */
        while (read(fd, &(linea[j]), 1) != 0)         /* leggiamo un carattere alla volta e lo inseriamo nell'array linea */
        {
                if (linea[j] == '\n')  /* se il carattere corrente e' il terminatore di linea ... */
                {
                        cur_tot++;      /* incrementiamo il conteggio delle linee */
                        if (cur_tot == min)     /* controlliamo se siamo sulla linea che dobbiamo selezionare */
                        {
				/* stampiamo la linea selezionata */
        			/* dobbiamo prima rendere la linea una stringa */
        			linea[j] = '\0';
        			/* poi la stampiamo su standard output */
        			printf("Sono il figlio di indice %d e PID %d e la linea numero %ld del file %s e': %s\n", n, getpid(), min, argv[n+1], linea);
                                break; /* usciamo dal ciclo di lettura */
                        }
                        else            /* se non siamo sulla linea giusta */
                        {
                                j = 0;  /* azzeriamo l'indice per la prossima linea */
                        }
                }
                else j++;       /* altrimenti incrementiamo l'indice di linea */
        }

	/* ogni figlio deve tornare il proprio indice d'ordine */
	exit(n);
	}
} /* fine for */

/* codice del padre */
/* chiusura pipe: tutte meno l'ultima in lettura */
for(n=0; n < N; n++)
{
	close(pipes[n][1]);
	if (n != N-1) close(pipes[n][0]);
}

/* il padre deve leggere la struttura che gli arriva dall'ultimo figlio */
nr=read(pipes[N-1][0],&letta,sizeof(s_tot));
/* come prima effettuaiamo il controllo sulla lettura */
if (nr != sizeof(s_tot))
{
        printf("Padre ha letto un numero di byte sbagliati %d\n", nr);
        exit(6); /* N.B. in questo caso il padre NON aspettera' i figli */ 
}
/* il padre deve stampare i campi della struttura ricevuta insieme con l'indicazione del pid del figlio e il file cui si riferiscono le informazioni ricevute */
printf("Il figlio di indice %d e pid %d ha trovato il numero minimo di linee %ld nel file %s\n", letta.id, pid[letta.id], letta.tot, argv[letta.id+1]);

/* ora il padre deve mandare a tutti i figli il valore del minimo calcolato dai figli e che ha ricevuto */
for (n=0; n < N; n++)
{
	write(pipe_pf[n][1], &letta.tot, sizeof(letta.tot));
}

/* Il padre aspetta i figli */
for (n=0; n < N; n++)
{
        pidFiglio = wait(&status);
        if (pidFiglio < 0)
        {
                printf("Errore in wait\n");
                exit(7);
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