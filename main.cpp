#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_COMMAND_LENGTH 1024
#define MAX_ARGS 64

// Arkaplanda çalışan işlemlerin bilgilerini tutan yapı
typedef struct {
    pid_t pid;
    int status;
} BackgroundProcess;

BackgroundProcess background_processes[64];
int background_count = 0;

// Komut istemini (prompt) yazdırır
void print_prompt() {
    printf("> ");
    fflush(stdout);
}

// Arka planda çalışan işlemleri kontrol eder ve bittiğinde bilgilerini yazdırır
void handle_background_processes() {
    for (int i = 0; i < background_count; i++) {
        pid_t result = waitpid(background_processes[i].pid, &background_processes[i].status, WNOHANG);
        if (result > 0) {
            printf("[pid: %d] retval: %d\n", background_processes[i].pid, WEXITSTATUS(background_processes[i].status));
        }
    }
}

// Tek bir komutun yürütülmesini sağlar
void execute_command(char **args, int background, int input_fd, int output_fd) {
    pid_t pid = fork();

    if (pid == 0) {
        // Çocuk proses: Gerekli yönlendirmeleri yap
        if (input_fd != STDIN_FILENO) {
            dup2(input_fd, STDIN_FILENO);
            close(input_fd);
        }
        if (output_fd != STDOUT_FILENO) {
            dup2(output_fd, STDOUT_FILENO);
            close(output_fd);
        }

        execvp(args[0], args);
        perror("Komut çalıştırılamadı");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Ebeveyn proses
        if (background) {
            background_processes[background_count].pid = pid;
            background_count++;
        } else {
            waitpid(pid, NULL, 0);
        }
    } else {
        perror("Fork başarısız");
    }
}

// Pipe işlemlerini uygular
void handle_pipes(char *input) {
    int pipe_fds[2], input_fd = STDIN_FILENO;
    char *commands[MAX_ARGS];
    int command_count = 0;

    // Komutları '|' ile böl
    char *token = strtok(input, "|");
    while (token != NULL) {
        commands[command_count++] = token;
        token = strtok(NULL, "|");
    }

    for (int i = 0; i < command_count; i++) {
        char *args[MAX_ARGS];
        int arg_count = 0;
        int output_fd = STDOUT_FILENO;

        // Komutları ve argümanlarını ayır
        token = strtok(commands[i], " \t\n");
        while (token != NULL) {
            args[arg_count++] = token;
            token = strtok(NULL, " \t\n");
        }
        args[arg_count] = NULL;

        if (i < command_count - 1) {
            // Yeni bir pipe oluştur
            pipe(pipe_fds);
            output_fd = pipe_fds[1];
        }

        execute_command(args, 0, input_fd, output_fd);

        if (input_fd != STDIN_FILENO) close(input_fd);
        if (output_fd != STDOUT_FILENO) close(output_fd);

        input_fd = pipe_fds[0];
    }
}

// Girdi komutunu ayrıştırır ve çalıştırır
void parse_and_execute(char *input) {
    char *args[MAX_ARGS];
    int background = 0;

    // Girişten arka plan operatörü & kontrolü
    if (strchr(input, '&')) {
        background = 1;
        *strchr(input, '&') = '\0';
    }

    if (strchr(input, '|')) {
        handle_pipes(input);
        return;
    }

    int input_fd = STDIN_FILENO, output_fd = STDOUT_FILENO;

    // Giriş ve çıkış yönlendirme kontrolü
    if (strchr(input, '<')) {
        char *infile = strchr(input, '<') + 1;
        *strchr(input, '<') = '\0';
        while (*infile == ' ') infile++;
        char *end = strpbrk(infile, " \n");
        if (end) *end = '\0';
        input_fd = open(infile, O_RDONLY);
        if (input_fd < 0) {
            perror("Giriş dosyası bulunamadı");
            return;
        }
    }

    if (strchr(input, '>')) {
        char *outfile = strchr(input, '>') + 1;
        *strchr(input, '>') = '\0';
        while (*outfile == ' ') outfile++;
        char *end = strpbrk(outfile, " \n");
        if (end) *end = '\0';
        output_fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (output_fd < 0) {
            perror("Çıkış dosyası açılamadı");
            return;
        }
    }

    // Komutları ayır
    char *token = strtok(input, " \t\n");
    int i = 0;
    while (token != NULL) {
        args[i++] = token;
        token = strtok(NULL, " \t\n");
    }
    args[i] = NULL;

    if (args[0] == NULL) {
        // Boş komut
        return;
    }

    if (strcmp(args[0], "quit") == 0) {
        // quit komutuyla kabuğu sonlandır
        if (background_count > 0) {
            // Arka planda işlem varsa bekle
            for (int i = 0; i < background_count; i++) {
                waitpid(background_processes[i].pid, &background_processes[i].status, 0);
                printf("[pid: %d] retval: %d\n", background_processes[i].pid, WEXITSTATUS(background_processes[i].status));
            }
        }
        exit(0);
    }

    execute_command(args, background, input_fd, output_fd);

    if (input_fd != STDIN_FILENO) close(input_fd);
    if (output_fd != STDOUT_FILENO) close(output_fd);
}

int main() {
    char command[MAX_COMMAND_LENGTH];

    // Ana döngü: Komutları al ve çalıştır
    while (1) {
        handle_background_processes(); // Arka plan işlemlerini kontrol et
        print_prompt(); // Prompt yazdır

        if (fgets(command, sizeof(command), stdin) == NULL) {
            break; // EOF veya hata durumunda çık
        }

        parse_and_execute(command); // Komutu ayrıştır ve çalıştır
    }

    return 0;
}
