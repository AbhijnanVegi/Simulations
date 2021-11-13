#if !defined(STD_STRUCTS_H)
#define STD_STRUCTS_H

#include <pthread.h>

//colors
#define RED "\033[0;31m"
#define GREEN "\033[0;32m"
#define YELLOW "\033[0;33m"
#define BLUE "\033[0;34m"
#define MAGENTA "\033[0;35m"
#define CYAN "\033[0;36m"
#define WHITE "\033[0;37m"
#define RESET "\033[0m"

#define H 0
#define A 1
#define N 2

typedef struct spec {
    char *name;
    int entry_time;
    int patience;
    int type;
    int goals;
    int seat_type;
} spec;

typedef struct grp {
    int id;
    int size;
    pthread_t *threads;
} grp;

#endif // STD_STRUCTS_H
