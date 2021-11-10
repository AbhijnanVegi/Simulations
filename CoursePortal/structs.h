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
    ta *tas;
    sem_t ta_avail;
} lab;

typedef struct course {
    char *name;
    double interest;
    int max_slot_c;
    int lab_c;
    int *labs;
    bool withdrawn;
    sem_t course_avail;
} course;

typedef struct student {
    double calibre;
    int pref1;
    int pref2;
    int pref3;
    int apply_time;
} student;