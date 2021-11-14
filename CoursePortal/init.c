#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#include "structs.h"
#include "init.h"

void init_ta(struct ta *ta)
{
    ta->ta_timed = 0;
    pthread_mutex_init(&ta->ta_lock, NULL);
}

void init_lab(struct lab *lab, char*name, int ta_c, int ta_times)
{
    lab->name = strdup(name);
    lab->ta_c = ta_c;
    lab->ta_times = ta_times;
    lab->ta_avail_c = ta_c;

    lab->tas = malloc(sizeof(struct ta) * ta_c);
    for (int i = 0; i < ta_c; i++)
    {
        init_ta(&lab->tas[i]);
    }
}

void init_course(
    course *course,
    char *name,
    double interest,
    int max_slot_c,
    int lab_c,
    int *labs
)
{
    course->name = strdup(name);
    course->interest = interest;
    course->max_slot_c = max_slot_c;
    course->lab_c = lab_c;
    course->labs = labs;
    course->withdrawn = false;

    sem_init(&course->interested, 0, 0);
    sem_init(&course->seats, 0, 0);
    pthread_mutex_init(&course->tut_lock, NULL);
    pthread_cond_init(&course->tut_cond, NULL);
}

void init_student(
    student *student,
    double calibre,
    int* prefs,
    int apply_time
)
{
    student->calibre = calibre;
    student->prefs = prefs;
    student->apply_time = apply_time;
}
