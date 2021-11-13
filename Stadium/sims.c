#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>

#include "structs.h"

#define H_CAT 0
#define A_CAT 1
#define N_CAT 2


int scoreboard[2] = {0};
pthread_mutex_t scoreboard_lock;
pthread_cond_t scoreboard_cond[2];

extern int seats[3];
char zones[] = {'H', 'A', 'N'};
pthread_mutex_t seat_lock;
pthread_cond_t seat_cats[3];

extern int spec_time;

void init_stad(void)
{
    pthread_mutex_init(&scoreboard_lock, NULL);
    pthread_cond_init(&scoreboard_cond[H], NULL);
    pthread_cond_init(&scoreboard_cond[A], NULL);

    pthread_mutex_init(&seat_lock, NULL);
    pthread_cond_init(&seat_cats[H_CAT], NULL);
    pthread_cond_init(&seat_cats[A_CAT], NULL);
    pthread_cond_init(&seat_cats[N_CAT], NULL);
}

void signal_cat(int type)
{
    if (type == A)
    {
        pthread_cond_signal(&seat_cats[A_CAT]);
    }
    else
    {
        pthread_cond_signal(&seat_cats[H_CAT]);
    }
    pthread_cond_signal(&seat_cats[N_CAT]);
}

void *seat_wait(void* sleep_arg)
{
    sleep_args* sp = (sleep_args*) sleep_arg;
    sleep(sp->time);
    pthread_mutex_lock(&seat_lock);
    pthread_mutex_lock(sp->exec_lock);
    pthread_cancel(sp->caller);
    pthread_mutex_unlock(sp->exec_lock);
    pthread_mutex_unlock(&seat_lock);
    printf(YELLOW"%s could not get a seat\n"RESET, sp->spec->name);
    printf(CYAN"%s is waiting for their friends at exit\n"RESET, sp->spec->name);
}

void *spec_wait(void * sleep_arg)
{
    sleep_args* sp = (sleep_args*) sleep_arg;
    sleep(sp->time);
    pthread_mutex_lock(&scoreboard_lock);
    pthread_mutex_lock(sp->exec_lock);
    pthread_cancel(sp->caller);
    pthread_mutex_unlock(sp->exec_lock);
    pthread_mutex_unlock(&scoreboard_lock);
    pthread_mutex_lock(&seat_lock);
    seats[sp->spec->seat_type]++;
    signal_cat(sp->spec->seat_type);
    pthread_mutex_unlock(&seat_lock);
    printf(YELLOW"%s watched the match for %d seconds and is leaving\n"RESET, sp->spec->name, sp->time);
    printf(CYAN"%s is waiting for their friends at exit\n"RESET, sp->spec->name);
}

void *spec_sim(void* spec_arg)
{
    spec* s = spec_arg;

    pthread_mutex_t exec_lock;
    pthread_mutex_init(&exec_lock, NULL);

    sleep(s->entry_time);
    printf(BLUE "%s has reached the stadium\n" RESET, s->name);
    
    // Setup thread for sleeping
    sleep_args* sleep_arg = malloc(sizeof(sleep_args));
    sleep_arg->time = s->patience;
    sleep_arg->caller = pthread_self();
    sleep_arg->exec_lock = &exec_lock;
    sleep_arg->spec = s;
    pthread_t sleep_thread;
    pthread_create(&sleep_thread, NULL, seat_wait, sleep_arg);

    // Wait for seat
    pthread_mutex_lock(&seat_lock);
    pthread_mutex_lock(&exec_lock);
    while (true)
    {

        if (s->type == A && seats[A])
        {
            seats[A]--;
            s->seat_type = A;
            break;
        }
        else if (s->type == H)
        {
            if (seats[H])
            {
                seats[H]--;
                s->seat_type = H;
                break;
            }
            else if (seats[N])
            {
                seats[N]--;
                s->seat_type = N;
                break;
            }
        }
        else
        {
            if (seats[N])
            {
                seats[N]--;
                s->seat_type = N;
                break;
            }
            else if (seats[H])
            {
                seats[H]--;
                s->seat_type = H;
                break;
            }
            else if (seats[A])
            {
                seats[A]--;
                s->seat_type = A;
                break;
            }
        }
        pthread_mutex_unlock(&exec_lock);
        pthread_cond_wait(&seat_cats[s->type], &seat_lock);
        pthread_mutex_lock(&exec_lock);
    }
    printf(GREEN "%s has got a seat in zone %c\n"RESET, s->name, zones[s->seat_type]);
    pthread_cancel(sleep_thread);
    pthread_mutex_unlock(&exec_lock);
    pthread_mutex_unlock(&seat_lock);

    // Setup thread for sleeping
    sleep_arg->time = spec_time;
    pthread_create(&sleep_thread, NULL, spec_wait, sleep_arg);

    // Wait for scoreboard
    pthread_mutex_lock(&scoreboard_lock);
    pthread_mutex_lock(&exec_lock);
    while (s->type == N || scoreboard[(s->type+1)%2] < s->goals)
    {
        pthread_mutex_unlock(&exec_lock);
        pthread_cond_wait(&scoreboard_cond[(s->type+1)%2], &scoreboard_lock);
        pthread_mutex_lock(&exec_lock);
    }
    printf(YELLOW "%s is leaving due to bad performance of his team\n" RESET, s->name);
    pthread_cancel(sleep_thread);
    pthread_mutex_unlock(&exec_lock);
    pthread_mutex_unlock(&scoreboard_lock);

    // Release the seat
    pthread_mutex_lock(&seat_lock);
    seats[s->seat_type]++;
    signal_cat(s->seat_type);
    pthread_mutex_unlock(&seat_lock);

    // Wait for friends at exit
    printf(CYAN "%s is waiting for their friends at the exit\n", s->name);
}

void *grp_sim(void* grp_arg)
{
    grp* g = (grp*) grp_arg;
    for (int i = 0; i < g->size;i++)
    {
        pthread_join(g->threads[i], NULL);
    }
    printf(MAGENTA "Group %d is leaving for dinner\n");
}
