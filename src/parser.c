/**
 * parser.c
 * Komut ayrıştırma fonksiyonlarının implementasyonu
 */

#include "parser.h"

void trimWhitespace(char* str) {
    char *start = str;
    while (*start == ' ' || *start == '\t') start++;
    
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }

    char *end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t')) {
        *end = '\0';
        end--;
    }
}

void freeStringArray(char** array, int count) {
    if (array) {
        for (int i = 0; i < count; i++) {
            if (array[i]) free(array[i]);
        }
        free(array);
    }
}

char** splitBySemicolon(char* line, int* count) {
    int maxCommands = 50;
    char** commands = malloc(sizeof(char*) * maxCommands);
    *count = 0;

    char* token = strtok(line, ";");
    while (token != NULL && *count < maxCommands) {
        commands[*count] = strdup(token);
        trimWhitespace(commands[*count]);
        (*count)++;
        token = strtok(NULL, ";");
    }
    commands[*count] = NULL;
    return commands;
}

char** splitByPipe(char* line, int* count) {
    int maxPipes = 10;
    char** cmds = malloc(sizeof(char*) * maxPipes);
    *count = 0;

    char* token = strtok(line, "|");
    while (token != NULL && *count < maxPipes) {
        cmds[*count] = strdup(token);
        trimWhitespace(cmds[*count]);
        (*count)++;
        token = strtok(NULL, "|");
    }
    cmds[*count] = NULL;
    return cmds;
}

char** parseSingleCommand(char* cmd, char** inputFile, char** outputFile, int* bg) {
    *inputFile = NULL;
    *outputFile = NULL;
    *bg = 0;

    int maxTokens = 50;
    char** args = malloc(sizeof(char*) * (maxTokens + 1));
    memset(args, 0, sizeof(char*) * (maxTokens + 1));

    int argc = 0;
    char* token = strtok(cmd, " \t\r\n");
    while (token != NULL && argc < maxTokens) {
        if (strcmp(token, "<") == 0) {
            token = strtok(NULL, " \t\r\n");
            if (token) *inputFile = strdup(token);
        } else if (strcmp(token, ">") == 0) {
            token = strtok(NULL, " \t\r\n");
            if (token) *outputFile = strdup(token);
        } else if (strcmp(token, "&") == 0) {
            *bg = 1;
            break;
        } else {
            args[argc++] = strdup(token);
        }
        token = strtok(NULL, " \t\r\n");
    }
    args[argc] = NULL;
    return args;
}