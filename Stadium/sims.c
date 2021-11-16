#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#include "structs.h"


int scoreboard[2] = {0};
pthread_mutex_t scoreboard_lock;
pthread_cond_t scoreboard_cond[2];

extern sem_t seats[3];
char zones[] = {'H', 'A', 'N'};

extern int spec_time;

typedef struct seat 
{
    int type;
    int *seat_type;
    pthread_mutex_t *lock;
    pthread_cond_t *cond;
    struct timespec *end_wait;
} seat;

void
init_stad(void)
{
    pthread_mutex_init(&scoreboard_lock, NULL);
    pthread_cond_init(&scoreboard_cond[H], NULL);
    pthread_cond_init(&scoreboard_cond[A], NULL);
}

void *seat_wait_sim(void* seat_arg)
{
    seat* s = (seat*) seat_arg;
    int rt;
    rt = sem_timedwait(&seats[s->type], s->end_wait);
    if (rt == 0)
    {
        pthread_mutex_lock(s->lock);
        if (*s->seat_type != E)
        {
            sem_post(&seats[s->type]);
            pthread_mutex_unlock(s->lock);
        }
        else
        {
            *(s->seat_type) = s->type;
            pthread_cond_signal(s->cond);
        }
        pthread_mutex_unlock(s->lock);
    }
}

void *spec_sim(void* spec_arg)
{
    spec* s = spec_arg;
    int rt;

    sleep(s->entry_time);
    printf(BLUE "%s has reached the stadium\n" RESET, s->name);

    // Set time to end waiting
    struct timespec end_wait;
    end_wait.tv_sec = time(NULL) + s->patience;
    end_wait.tv_nsec = 0;

    // Wait for seat
    pthread_mutex_t seat_lock;
    pthread_mutex_init(&seat_lock, NULL);
    pthread_cond_t seat_cond;

    pthread_mutex_lock(&seat_lock);
    if (s->type == A || s->type == N)
    {
        seat* a = malloc(sizeof(seat));
        a->type = A;
        a->lock = &seat_lock;
        a->cond = &seat_cond;
        a->end_wait = &end_wait;
        a->seat_type = &(s->seat_type);
        pthread_t at;
        rt = pthread_create(&at, NULL, seat_wait_sim, a);
    }
    if (s->type == H || s->type == N)
    {
        seat *h = malloc(sizeof(seat));
        h->type = H;
        h->lock = &seat_lock;
        h->cond = &seat_cond; 
        h->end_wait = &end_wait;
        h->seat_type = &(s->seat_type);
        seat *n = malloc(sizeof(seat));
        n->type = N;
        n->lock = &seat_lock;
        n->cond = &seat_cond;
        n->end_wait = &end_wait;
        n->seat_type = &(s->seat_type);
        pthread_t ht, nt;
        rt = pthread_create(&ht, NULL, seat_wait_sim, h);
        rt = pthread_create(&nt, NULL, seat_wait_sim, n);
    }
    rt = pthread_cond_timedwait(&seat_cond, &seat_lock, &end_wait);
    if (s->seat_type == E)
    {
        s->seat_type = X;
        pthread_mutex_unlock(&seat_lock);
        goto noseat;
    }
    pthread_mutex_unlock(&seat_lock);
    
    printf(GREEN "%s has got a seat in zone %c\n"RESET, s->name, zones[s->seat_type]);

    // Set endtime
    end_wait.tv_sec = time(NULL) + spec_time;
    // Wait for scoreboard
    pthread_mutex_lock(&scoreboard_lock);
    while (s->type == N || scoreboard[(s->type+1)%2] < s->goals)
    {
        rt = pthread_cond_timedwait(&scoreboard_cond[(s->type+1)%2], &scoreboard_lock, &end_wait);
        if (rt != 0)
        {
            goto endspec;
        }
    }
    printf(YELLOW "%s is leaving due to bad performance of his team\n" RESET, s->name);
    pthread_mutex_unlock(&scoreboard_lock);

    // Release the seat
    sem_post(&seats[s->seat_type]);

    // Wait for friends at exit
    printf(CYAN "%s is waiting for their friends at the exit\n", s->name);
    pthread_exit(NULL);

noseat:;
    printf(YELLOW"%s couldn't get a seat\n"RESET, s->name);
    printf(CYAN"%s is waiting for their friends at the exit\n" RESET, s->name);
    pthread_exit(NULL);
endspec:;
    printf(YELLOW "%s watched the match for %d seconds and is leaving\n"RESET, s->name, spec_time);
    pthread_mutex_unlock(&scoreboard_lock);

    // Release the seat
    sem_post(&seats[s->seat_type]);

    // Wait for friends at exit
    printf(CYAN "%s is waiting for their friends at the exit\n"RESET, s->name);
    pthread_exit(NULL);
}

void *grp_sim(void* grp_arg)
{
    grp* g = (grp*) grp_arg;
    for (int i = 0; i < g->size;i++)
    {
        pthread_join(g->threads[i], NULL);
    }
    printf(MAGENTA "Group %d is leaving for dinner\n"RESET, g->id);
}


