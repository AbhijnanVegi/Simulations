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
    while (!c->stu_wait_c);
    #ifdef DEBUG
    printf("Course %s: %d Students registered for course\n", c->name, c->stu_wait_c);
    #endif
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
                for (int j = 0;j < ilabs[c->labs[i]].ta_c; j++)
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
        pthread_mutex_lock(&c->course_lock);
        #ifdef DEBUG
        printf("DEBUG: Course %s locked course\n", c->name);
        #endif
        pthread_cond_broadcast(&c->course_cond);
        pthread_mutex_unlock(&c->course_lock);
        #ifdef DEBUG
        printf("DEBUG: Course %s unlocked course\n", c->name);
        #endif
        printf("Course %s doesn't have any TA's eligible and is removed from course offerings\n", c->name);
        pthread_exit(NULL);
    }

// TA found
found:;
    lab *l = &ilabs[lab_no];
    ta *t = &l->tas[ta_no];
    // Update ta stats
    t->ta_timed++;
    printf("TA %d from lab %s has been allocated to course %s for %d TA ship\n",ta_no,l->name,c->name,t->ta_timed);
    if (t->ta_timed == l->ta_times)
    {
        l->ta_avail_c--;
        if (l->ta_avail_c == 0)
        {
            printf("Lab %s no longer has students available for TA ship\n", l->name);
        }
    }
    // Allocate seats
    int seats_allocated = rand() % c->max_slot_c + 1;
    c->tut_wait_c = 0;
    printf("Course %s has been allocated %d seats\n", c->name, seats_allocated);

    // Wakeup seats_allocated students
    pthread_mutex_lock(&c->course_lock);
    #ifdef DEBUG
    printf("DEBUG: Course %s locked course\n", c->name);
    #endif
    int available_students = c->stu_wait_c;
    for (int i = 0; i < seats_allocated;i++)
    {
        pthread_cond_signal(&c->course_cond);
    }
    pthread_mutex_unlock(&c->course_lock);
#ifdef DEBUG
        printf("DEBUG: Course %s unlocked course\n", c->name);
    #endif
    // Wait for allocated students to join
    while (c->tut_wait_c != seats_allocated && c->tut_wait_c != available_students);

    // Run tutorial
    printf("Tutorial has started for Course %s with %d seats filled out of %d\n", c->name, c->tut_wait_c, seats_allocated);
    sleep(1);

    // Broadcast end of tutorial
    pthread_mutex_lock(&c->tut_lock);
    #ifdef DEBUG
        printf("DEBUG: Course %s locked tut\n", c->name);
    #endif
    printf("TA %d from lab %s has completed the tutorial and the left the course %s\n", ta_no, l->name, c->name);
    pthread_cond_broadcast(&c->tut_cond);
    pthread_mutex_unlock(&c->tut_lock);
    #ifdef DEBUG
        printf("DEBUG: Course %s unlocked tut\n", c->name);
    #endif

    // Release TA
    if (t->ta_timed != l->ta_times)
    {
        pthread_mutex_unlock(&t->ta_lock);
    }
    goto search;
}

void* student_sim(void* stu_arg)
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
        // Register as waiting
        pthread_mutex_lock(&c->course_lock);
        #ifdef DEBUG
        printf("DEBUG: Student %d locked course %s\n", stu_no, c->name);
        #endif
        c->stu_wait_c++;

        // Wait for course to be available
        pthread_cond_wait(&c->course_cond, &c->course_lock);
        #ifdef DEBUG
        printf("DEBUG: Student %d finished waiting for course %s\n", stu_no, c->name);
        #endif
        c->stu_wait_c--;
        c->tut_wait_c++;
        // Check again for course withdrawn
        if (c->withdrawn)
        {
            pthread_mutex_unlock(&c->course_lock);
            goto change_prio;
        }
        printf("Student %d has been allocated a seat in course %s\n", stu_no, c->name);
        pthread_mutex_unlock(&c->course_lock);
        #ifdef DEBUG
        printf("DEBUG: Student %d unlocked course %s\n", stu_no, c->name);
        #endif

        // Wait for tutorial
        pthread_mutex_lock(&c->tut_lock);
        #ifdef DEBUG
        printf("DEBUG: Student %d locked tut\n", stu_no);
        #endif
        pthread_cond_wait(&c->tut_cond, &c->tut_lock);
        #ifdef DEBUG
        printf("DEBUG: Student %d finished waiting for tut\n", stu_no);
        #endif
        pthread_mutex_unlock(&c->tut_lock);
        #ifdef DEBUG
        printf("DEBUG: Student %d unlocked tut\n", stu_no);
        #endif


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
        if ( i != 2)
            printf("Student %d has changed current preference from course %s (priority %d) to course %s (priority %d)\n", stu_no, c->name, i, courses[stu->prefs[i + 1]].name, i + 1);
    }
    // Student exits the simulation
    printf("Student %d either didn't get any of their preferred courses or has withdrawn from them and exited the simulation\n", stu_no);
    pthread_exit(NULL);
}