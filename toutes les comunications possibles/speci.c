#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctype.h>

#define MAX_CARATTERI 1000  // Buffer per lettura dei caratteri

int main(int argc, char *argv[]) {
    int N;              // Numero di file/processi
    int n;              // Indice dei processi figli
    int pid;            // PID dei processi figli
    int fd;             // File descriptor per aprire i file
    long int somma;     // Variabile per la somma dei numeri letti dai file
    char car;           // Carattere letto dai file
    int caratteriNumerici; // Conteggio dei caratteri numerici nel file
    int stat;           // Stato del processo figlio
    int nr;             // Numero di caratteri letti da read()

    if (argc < 3) {
        fprintf(stderr, "Uso: %s <file1> <file2> ... <fileN> <n>\n", argv[0]);
        exit(1);
    }

    N = argc - 2;  // Numero di file/processi
    int ultimaPosizione = N - 1;

    for (n = 0; n < N; n++) {
        if ((pid = fork()) < 0) {
            perror("fork");
            exit(2);
        }

        if (pid == 0) {  // Codice del processo figlio
            somma = 0;
            caratteriNumerici = 0;

            if ((fd = open(argv[n+1], O_RDONLY)) < 0) {
                perror("open");
                exit(-1);
            }

            while ((nr = read(fd, &car, 1)) > 0) {
                if (isdigit(car)) {
                    somma += (car - '0');  // Converti carattere in numero e aggiungi alla somma
                    caratteriNumerici++;
                }
            }

            close(fd);

            // Comunica la somma al padre
            write(STDOUT_FILENO, &somma, sizeof(somma));

            // Ritorna il numero di caratteri numerici trovati
            exit(caratteriNumerici);
        }
    }

    // Codice del processo padre
    for (n = 0; n < N; n++) {
        long int sommaFiglio;
        int numCaratteri;

        // Legge la somma dal figlio
        if (read(STDIN_FILENO, &sommaFiglio, sizeof(sommaFiglio)) < 0) {
            perror("read");
            exit(3);
        }

        // Stampa la somma
        printf("File %d: %s, somma = %ld\n", n, argv[n+1], sommaFiglio);

        // Attende il termine del processo figlio e ottiene il valore di ritorno
        pid = wait(&stat);
        if (pid < 0) {
            perror("wait");
            exit(4);
        }
        if (WIFEXITED(stat)) {
            numCaratteri = WEXITSTATUS(stat);
            printf("Figlio con PID %d ha trovato %d caratteri numerici\n", pid, numCaratteri);
        } else {
            fprintf(stderr, "Il processo figlio con PID %d è terminato con errore\n", pid);
        }
    }

    return 0;
}
