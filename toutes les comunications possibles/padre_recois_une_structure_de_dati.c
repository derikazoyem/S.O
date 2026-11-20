#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>

#define MAX_FILENAME_LENGTH 510

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <file1> <file2> ... <fileN>\n", argv[0]);
        exit(1);
    }

    int N = argc - 1;
    pid_t pids[N], npids[N];
    int status;
    int pipefd[N][2];

    for (int j = 0; j < N; j++) {
        if (pipe(pipefd[j]) == -1) {
            perror("pipe");
            exit(1);
        }

        if ((pids[j] = fork()) == 0) {
            // Processo figlio
            char out_filename[MAX_FILENAME_LENGTH];
            snprintf(out_filename, MAX_FILENAME_LENGTH, "inverso%d", j);

            int out_fd = open(out_filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (out_fd == -1) {
                perror("open output file");
                exit(1);
            }

            int in_fd = open(argv[j + 1], O_RDONLY);
            if (in_fd == -1) {
                perror("open input file");
                exit(1);
            }

            off_t file_size = lseek(in_fd, 0, SEEK_END);
            off_t mid = file_size / 2;

            if ((npids[j] = fork()) == 0) {
                // Processo nipote
                off_t start = file_size - 1;
                off_t end = mid;

                while (start >= end) {
                    lseek(in_fd, start, SEEK_SET);
                    char ch;
                    read(in_fd, &ch, 1);
                    write(out_fd, &ch, 1);
                    start--;
                }
                close(in_fd);
                close(out_fd);
                _exit(mid);
            } else {
                waitpid(npids[j], &status, 0);
                int bytes_read_by_np = WEXITSTATUS(status);

                off_t start = mid - 1;
                off_t end = 0;

                while (start >= end) {
                    lseek(in_fd, start, SEEK_SET);
                    char ch;
                    read(in_fd, &ch, 1);
                    write(out_fd, &ch, 1);
                    start--;
                }

                close(in_fd);
                close(out_fd);

                write(pipefd[j][1], &bytes_read_by_np, sizeof(bytes_read_by_np));
                _exit(file_size);
            }
        }
    }

    for (int j = 0; j < N; j++) {
        waitpid(pids[j], &status, 0);
        int bytes_read_by_child;
        read(pipefd[j][0], &bytes_read_by_child, sizeof(bytes_read_by_child));
        printf("PID Figlio: %d, Bytes Lett/scritti: %d\n", pids[j], bytes_read_by_child);

        char out_filename[MAX_FILENAME_LENGTH];
        snprintf(out_filename, MAX_FILENAME_LENGTH, "inverso%d", j);
        printf("File: %s\n", out_filename);

        int out_fd = open(out_filename, O_RDONLY);
        if (out_fd == -1) {
            perror("open output file");
            exit(1);
        }

        char buf;
        while (read(out_fd, &buf, 1) > 0) {
            write(STDOUT_FILENO, &buf, 1);
        }
        close(out_fd);
        printf("\n");
    }

    return 0;
}
