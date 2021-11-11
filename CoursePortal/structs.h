#if !defined(CP_STRUCTS_H)
#define CP_STRUCTS_H

#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>

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