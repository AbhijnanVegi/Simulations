#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <unistd.h>

#include "globals.h"
#include "structs.h"

void *course_sim(void *course_arg)
{
    course *c = (course *)course_arg;
    int lab_no = 0, ta_no = 0;
search:;
    // Wait for students to register for course
    sem_wait(&c->interested);
    sem_post(&c->interested);
    // Search for TAs
    bool has_tas = true;
    while (has_tas)
    {
        has_tas = false;
        for (int i = 0; i < c->lab_c; i++)
        {
            if (ilabs[c->labs[i]].ta_avail_c > 0)
            {
                has_tas = true;
                for (int j = 0; j < ilabs[c->labs[i]].ta_c; j++)
                {
                    int ret = pthread_mutex_trylock(&(ilabs[c->labs[i]].tas[j].ta_lock));
                    if (!ret)
                    {
                        lab_no = c->labs[i];
                        ta_no = j;
                        goto found;
                    }
                }
            }
        }
    }
    // If no TAs will be available withdraw
    if (!has_tas)
    {
        c->withdrawn = true;
        for (int i = 0; i < student_c; i++)
        {
            sem_post(&c->seats);
        }
        printf("Course %s doesn't have any TA's eligible and is removed from course offerings\n", c->name);
        pthread_exit(NULL);
    }

// TA found
found:;
    lab *l = &ilabs[lab_no];
    ta *t = &l->tas[ta_no];
    // Update ta stats
    t->ta_timed++;
    printf("TA %d from lab %s has been allocated to course %s for %d TA ship\n", ta_no, l->name, c->name, t->ta_timed);
    if (t->ta_timed == l->ta_times)
    {
        l->ta_avail_c--;
    }
    // Allocate seats
    int seats_allocated = rand() % c->max_slot_c + 1;
    c->tut_wait_c = 0;
    printf("Course %s has been allocated %d seats\n", c->name, seats_allocated);

    int available_students;
    sem_getvalue(&c->interested, &available_students);
    // Wakeup seats_allocated students
    for (int i = 0; i < (seats_allocated > available_students? available_students:seats_allocated); i++)
    {
        sem_post(&c->seats);
    }

    // Wait for allocated students to join
    while (c->tut_wait_c != seats_allocated && c->tut_wait_c != available_students)
        ;
    // Run tutorial
    printf("Tutorial has started for Course %s with %d seats filled out of %d\n", c->name, c->tut_wait_c, seats_allocated);
    sleep(1);

    // Broadcast end of tutorial
    pthread_mutex_lock(&c->tut_lock);

    printf("TA %d from lab %s has completed the tutorial and the left the course %s\n", ta_no, l->name, c->name);
    pthread_cond_broadcast(&c->tut_cond);
    pthread_mutex_unlock(&c->tut_lock);

    // Unregister students
    for (int i = 0; i < c->tut_wait_c; i++)
    {
        sem_wait(&c->interested);
    }

    // Release TA
    if (t->ta_timed != l->ta_times)
    {
        pthread_mutex_unlock(&t->ta_lock);
    }
    goto search;
}

void *student_sim(void *stu_arg)
{
    student *stu = (student *)stu_arg;
    int stu_no = stu - students;

    // Sleep till registration
    sleep(stu->apply_time);

    // Register
    printf("Student %d has filled in preferences for course registration\n", stu_no);

    // Wait for courses
    for (int i = 0; i < 3; i++)
    {
        course *c = &courses[stu->prefs[i]];
        if (c->withdrawn)
        {
            goto change_prio;
        }
        // Register as interested
        sem_post(&c->interested);
        // Wait for course to be available
        sem_wait(&c->seats);
        // Check again for course withdrawn
        if (c->withdrawn)
        {
            goto change_prio;
        }
        printf("Student %d has been allocated a seat in course %s\n", stu_no, c->name);

        // Wait for tutorial
        pthread_mutex_lock(&c->tut_lock);
        c->tut_wait_c++;
        pthread_cond_wait(&c->tut_cond, &c->tut_lock);
        pthread_mutex_unlock(&c->tut_lock);

        double prob = c->interest * stu->calibre;
        if (rand() % 100 < prob * 100)
        {
            printf("Student %d has selected course %s permanently\n", stu_no, c->name);
            pthread_exit(NULL);
        }
        else
        {
            goto change_prio;
        }
    change_prio:;
        if (i != 2)
            printf("Student %d has changed current preference from course %s (priority %d) to course %s (priority %d)\n", stu_no, c->name, i, courses[stu->prefs[i + 1]].name, i + 1);
    }
    // Student exits the simulation
    printf("Student %d either didn't get any of their preferred courses or has withdrawn from them and exited the simulation\n", stu_no);
    pthread_exit(NULL);
}

void* lab_sim(void* lab_arg)
{
    lab *l = (lab *)lab_arg;
    while (l->ta_avail_c)
        ;
    printf("Lab %s has no students available for TA ship\n", l->name);
    pthread_exit(NULL);
}
