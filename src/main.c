/**
 * main.c
 * Shell programının giriş noktası ve ana döngüsü
 */

#include "../include/header.h"

int main(int argc, char* argv[]) {
    // SIGCHLD sinyali için handler kurulumu
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigchldHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    // Ana kabuk döngüsü
    while (1) {
        printf("> ");
        fflush(stdout);

        char line[MAX_LINE];
        if (fgets(line, MAX_LINE, stdin) == NULL) {
            printf("\n");
            break;
        }

        if (line[0] == '\n') continue;
        line[strcspn(line, "\n")] = '\0';

        executeLine(line);
    }

    // Arka plan işlemlerinin bitmesini bekle
    if (bgProcessCount > 0) {
        printf("Waiting for background processes to complete...\n");
        while (bgProcessCount > 0) {
            wait(NULL);
        }
    }

    return 0;
}