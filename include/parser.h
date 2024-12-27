/**
 * parser.h
 * Komut ayrıştırma fonksiyonları için header dosyası
 */

#ifndef SHELL_PARSER_H
#define SHELL_PARSER_H

#include "header.h"

// Parser fonksiyonları için yardımcı fonksiyonlar
void trimWhitespace(char* str);
void freeStringArray(char** array, int count);

#endif // SHELL_PARSER_H