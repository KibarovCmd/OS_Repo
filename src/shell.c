/****************************************************************************
 * shell.c
 * 
 * Shell Uygulaması - Çekirdek İşlevler
 * 
 * Bu dosya, shell'in temel işlevlerini gerçekleştiren fonksiyonları içerir:
 * - Komut yürütme
 * - Süreç (process) yönetimi
 * - Boru hattı (pipeline) işlemleri
 * - Arka plan işlem yönetimi
 * 
 ****************************************************************************/

#include "../include/header.h"
#include "../include/parser.h"

/* Arka planda çalışan işlem sayısını tutan global değişken */
atomic_int bgProcessCount = 0;

/**
 * SIGCHLD Sinyal İşleyicisi
 * 
 * Bir çocuk süreç sonlandığında çağrılır ve:
 * - Sürecin çıkış durumunu kontrol eder
 * - Arka plan işlem sayacını günceller
 * - Sürecin PID ve çıkış kodunu ekrana yazdırır
 */
void sigchldHandler(int sig) {
    /* errno'yu koru - sinyal işleyicilerinde önemli */
    int saved_errno = errno;
    int status;
    pid_t pid;

    /* 
     * Tamamlanan tüm çocuk süreçleri topla
     * WNOHANG: Bekleyen süreç yoksa hemen dön
     */
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (bgProcessCount > 0) {
            bgProcessCount--;
            char msg[100];
            
            /* Sürecin nasıl sonlandığını kontrol et */
            if (WIFEXITED(status)) {
                /* Normal çıkış durumu */
                snprintf(msg, sizeof(msg), "[%d] retval: %d\n", 
                        pid, WEXITSTATUS(status));
            } else if (WIFSIGNALED(status)) {
                /* Sinyal ile sonlandırılma durumu */
                snprintf(msg, sizeof(msg), "[%d] killed by signal %d\n", 
                        pid, WTERMSIG(status));
            }
            
            /* Mesajı ekrana yaz (printf yerine write kullan - sinyal güvenliği için) */
            write(STDOUT_FILENO, msg, strlen(msg));
        }
    }
    
    /* Korunan errno değerini geri yükle */
    errno = saved_errno;
}

/**
 * Boru Hattı Yürütücü
 * 
 * Birden fazla komutu boru hattı ile birbirine bağlayarak çalıştırır.
 * Örnek: cmd1 | cmd2 | cmd3
 * 
 * @param pipelineCommands Boru hattındaki komutlar dizisi
 * @param pipeCount Komut sayısı
 */
void executePipeline(char** pipelineCommands, int pipeCount) {
    int pipes[2][2];  /* İki set boru tanımlayıcısı */
    int current = 0;  /* Aktif boru indeksi */
    
    /* Her bir komut için işlem yap */
    for (int i = 0; i < pipeCount; i++) {
        char *inputFile, *outputFile;
        int bg;
        
        /* Komutu ayrıştır */
        char** args = parseSingleCommand(pipelineCommands[i], &inputFile, &outputFile, &bg);
        
        /* Son komut değilse yeni boru oluştur */
        if (i < pipeCount - 1) {
            if (pipe(pipes[current]) == -1) {
                perror("pipe");
                exit(EXIT_FAILURE);
            }
        }

        /* Yeni süreç oluştur */
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) {  /* Çocuk süreç */
            /* İlk komut değilse, önceki borudan girdi al */
            if (i > 0) {
                dup2(pipes[1-current][0], STDIN_FILENO);
                close(pipes[1-current][0]);
                close(pipes[1-current][1]);
            }

            /* Son komut değilse, çıktıyı sonraki boruya yönlendir */
            if (i < pipeCount - 1) {
                dup2(pipes[current][1], STDOUT_FILENO);
                close(pipes[current][0]);
                close(pipes[current][1]);
            }

            /* İlk komut için dosyadan girdi yönlendirmesi */
            if (inputFile && i == 0) {
                int inFd = open(inputFile, O_RDONLY);
                if (inFd < 0) {
                    fprintf(stderr, "Input file not found.\n");
                    exit(EXIT_FAILURE);
                }
                dup2(inFd, STDIN_FILENO);
                close(inFd);
            }
            
            /* Son komut için dosyaya çıktı yönlendirmesi */
            if (outputFile && i == pipeCount - 1) {
                int outFd = open(outputFile, O_WRONLY|O_CREAT|O_TRUNC, 0666);
                if (outFd < 0) {
                    perror("open outputFile");
                    exit(EXIT_FAILURE);
                }
                dup2(outFd, STDOUT_FILENO);
                close(outFd);
            }

            /* Komutu çalıştır */
            execvp(args[0], args);
            perror("execvp");
            exit(EXIT_FAILURE);
        }

        /* Ana süreç - kullanılmayan boruları kapat */
        if (i > 0) {
            close(pipes[1-current][0]);
            close(pipes[1-current][1]);
        }

        /* Diğer boru setine geç */
        current = 1 - current;

        /* Bellek temizliği */
        freeStringArray(args, 50);
        if (inputFile) free(inputFile);
        if (outputFile) free(outputFile);
    }

    /* Tüm çocuk süreçlerin bitmesini bekle */
    for (int i = 0; i < pipeCount; i++) {
        wait(NULL);
    }
}

/**
 * Komut Satırı Yürütücü
 * 
 * Verilen komut satırını işler. Komut satırı şunları içerebilir:
 * - Tek bir komut
 * - Boru hattı ile bağlanmış komutlar
 * - Noktalı virgül ile ayrılmış çoklu komutlar
 * 
 * @param line İşlenecek komut satırı
 */
void executeLine(char* line) {
    /* Komut uzunluk kontrolü */
    if (strlen(line) >= MAX_LINE - 1) {
        fprintf(stderr, "Command too long\n");
        return;
    }

    int commandCount = 0;
    /* Noktalı virgülle ayrılmış komutları böl */
    char** semicolonCommands = splitBySemicolon(line, &commandCount);

    /* Her bir komutu işle */
    for (int i = 0; i < commandCount; i++) {
        /* Çıkış komutu kontrolü */
        if (strcmp(semicolonCommands[i], "quit") == 0) {
            if (bgProcessCount > 0) {
                printf("Waiting for %d background processes to complete...\n", 
                       bgProcessCount);
                while (bgProcessCount > 0) {
                    pause();  /* Arka plan işlemlerini bekle */
                }
            }
            exit(0);
        }

        /* Boru işareti kontrolü */
        int pipeCount = 0;
        char** pipelineCommands = splitByPipe(semicolonCommands[i], &pipeCount);

        if (pipeCount == 1) {  /* Tek komut */
            char *inputFile, *outputFile;
            int bg;
            char** args = parseSingleCommand(pipelineCommands[0], 
                                           &inputFile, &outputFile, &bg);

            /* Boş komut kontrolü */
            if (args[0] == NULL) {
                freeStringArray(args, 50);
                freeStringArray(pipelineCommands, pipeCount);
                continue;
            }

            /* Yeni süreç oluştur */
            pid_t pid = fork();
            if (pid < 0) {
                perror("fork");
            } else if (pid == 0) {  /* Çocuk süreç */
                /* Girdi yönlendirmesi */
                if (inputFile) {
                    int inFd = open(inputFile, O_RDONLY);
                    if (inFd < 0) {
                        fprintf(stderr, "Input file not found.\n");
                        exit(EXIT_FAILURE);
                    }
                    dup2(inFd, STDIN_FILENO);
                    close(inFd);
                }
                /* Çıktı yönlendirmesi */
                if (outputFile) {
                    int outFd = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
                    if (outFd < 0) {
                        perror("open");
                        exit(EXIT_FAILURE);
                    }
                    dup2(outFd, STDOUT_FILENO);
                    close(outFd);
                }

                /* Komutu çalıştır */
                execvp(args[0], args);
                perror("execvp");
                exit(EXIT_FAILURE);
            } else {  /* Ana süreç */
                if (bg == 1) {
                    /* Arka plan işlemi */
                    bgProcessCount++;
                } else {
                    /* Ön plan işlemi - tamamlanmasını bekle */
                    waitpid(pid, NULL, 0);
                }
            }

            /* Bellek temizliği */
            freeStringArray(args, 50);
            if (inputFile) free(inputFile);
            if (outputFile) free(outputFile);
        } else {
            /* Boru hattı işleme */
            executePipeline(pipelineCommands, pipeCount);
        }

        /* Pipeline komutları için bellek temizliği */
        freeStringArray(pipelineCommands, pipeCount);
    }

    /* Noktalı virgülle ayrılmış komutlar için bellek temizliği */
    freeStringArray(semicolonCommands, commandCount);
}