#if !defined(CP_STRUCTS_H)
#define CP_STRUCTS_H

#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>

#define BLACK "\x1b[30m"
#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define YELLOW "\x1b[33m"
#define BLUE "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN "\x1b[36m"
#define WHITE "\x1b[37m"
#define CRIMSON "\x1b[38m"
#define RESET "\x1b[0m"

typedef struct ta {
    int ta_timed;
    pthread_mutex_t ta_lock;
} ta;

typedef struct lab
{
    char *name;
    int ta_c;
    int ta_times;
    int ta_avail_c;
    ta *tas;
    pthread_mutex_t lab_lock;
} lab;

typedef struct course {
    char *name;
    double interest;
    int max_slot_c;
    int lab_c;
    int *labs;
    int stu_wait_c;
    int tut_wait_c;
    bool withdrawn;
    pthread_mutex_t course_lock;
    pthread_cond_t course_cond;
    pthread_mutex_t tut_lock;
    pthread_cond_t tut_cond;
} course;

typedef struct student {
    double calibre;
    int* prefs;
    int apply_time;
} student;
#endif // CP_STRUCTS_H