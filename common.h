#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
//extern char const * program_name;

void error(const char* cause, const char* message) {
    /* Запись сообщения об ошибке в поток stderr.program_name, */
    fprintf(stderr, ": error: (%s) %s\n", 
            cause, message);
    /* Завершение программы */
    exit(1);
}

void system_error(const char* operation) {
    /* Вывод сообщения об ошибке на основании значения
    переменной errno. */
    error(operation, strerror(errno));
}

#define TCP_PORT (1234)
#define END_MARK "end"
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))