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

    sem_init(&lab->ta_avail, 0, ta_c);
    
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

    sem_init(&course->course_avail, 0, 0);
}

void init_student(
    student *student,
    double calibre,
    int pref1,
    int pref2,
    int pref3,
    int apply_time
)
{
    student->calibre = calibre;
    student->pref1 = pref1;
    student->pref2 = pref2;
    student->pref3 = pref3;
    student->apply_time = apply_time;
}
