
#ifndef SHELL_HEADER_H
#define SHELL_HEADER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <stdatomic.h>

#define MAX_LINE 1024

// Global değişkenler
extern atomic_int bgProcessCount;

// Parser fonksiyonları
char** splitBySemicolon(char* line, int* count);
char** splitByPipe(char* line, int* count);
char** parseSingleCommand(char* cmd, char** inputFile, char** outputFile, int* bg);

// Pipeline yönetimi
void executePipeline(char** pipelineCommands, int pipeCount);

// Ana shell fonksiyonları
void executeLine(char* line);
void sigchldHandler(int sig);

#endif // SHELL_HEADER_H