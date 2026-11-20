#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define PERM 0644
#define MAX_LINE_LENGTH 250

typedef int pipe_t[2];

int main(int argc, char **argv) {
    int N = argc - 2;
    int nroLinee = atoi(argv[argc - 1]);
    int pid, fd, fdw, j, k, n;
    pipe_t *pipes;
    char buffer[MAX_LINE_LENGTH];
    int nr, nw;
    int status, ritorno;
    
    if (N < 1 || nroLinee < 1) {
        printf("Errore nel numero di parametri\n");
        exit(1);
    }

    pipes = (pipe_t *)malloc(N * sizeof(pipe_t));
    if (pipes == NULL) {
        printf("Errore allocazione pipe\n");
        exit(2);
    }

    for (n = 0; n < N; n++) {
        if (pipe(pipes[n]) < 0) {
            printf("Errore creazione pipe\n");
            exit(3);
        }
    }

    if ((fdw = creat("LEONARDI", PERM)) < 0) {
        printf("Errore nella creat del file %s\n", "LEONARDI");
        exit(4);
    }

    for (n = 0; n < N; n++) {
        if ((pid = fork()) < 0) {
            printf("Errore creazione figlio\n");
            exit(5);
        }
        if (pid == 0) {
            printf("Sono il figlio %d e sono associato al file %s\n", getpid(), argv[n + 1]);

            for (j = 0; j < N; j++) {
                if (j != n)
                    close(pipes[j][1]);
                if (n == N - 1 || j != n + 1)
                    close(pipes[j][0]);
            }

            if ((fd = open(argv[n + 1], O_RDONLY)) < 0) {
                printf("Impossibile aprire il file %s\n", argv[n + 1]);
                exit(-1);
            }

            for (int line_count = 0; line_count < nroLinee; line_count++) {
                k = 0;
                while (read(fd, &buffer[k], 1) > 0) {
                    if (buffer[k] == '\n') {
                        k++;
                        break;
                    }
                    k++;
                }
                buffer[k] = '\0';

                if (n != N - 1) {
                    nr = read(pipes[n + 1][0], buffer + k, MAX_LINE_LENGTH - k);
                    if (nr < 0) {
                        printf("Errore lettura da pipe\n");
                        exit(-1);
                    }
                    k += nr;
                }

                nw = write(pipes[n][1], buffer, k);
                if (nw != k) {
                    printf("Errore scrittura su pipe\n");
                    exit(-1);
                }
            }

            close(fd);
            exit(k);
        }
    }

    for (n = 0; n < N; n++) {
        close(pipes[n][1]);
        if (n != 0) close(pipes[n][0]);
    }

    for (j = 0; j < nroLinee; j++) {
        nr = read(pipes[0][0], buffer, MAX_LINE_LENGTH);
        if (nr < 0) {
            printf("Errore lettura da pipe\n");
            exit(6);
        }
        write(fdw, buffer, nr);
    }

    for (n = 0; n < N; n++) {
        pid = wait(&status);
        if (pid < 0) {
            printf("Errore in wait\n");
            exit(7);
        }
        if ((status & 0xFF) != 0)
            printf("Figlio con pid %d terminato in modo anomalo\n", pid);
        else {
            ritorno = (int)((status >> 8) & 0xFF);
            printf("Il figlio con pid=%d ha ritornato %d (se 255 problemi)\n", pid, ritorno);
        }
    }

    close(fdw);
    free(pipes);
    exit(0);
}
