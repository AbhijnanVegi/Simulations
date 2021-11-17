#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <assert.h>

#include "structs.h"

// Max spectators in stadium
#define MAX_SPECS 128

int scoreboard[2] = {0};

extern sem_t seats[3];
char zones[] = {'H', 'A', 'N'};

extern int spec_time; // Max spectating time
extern int types[]; // Conversion from char to int

// Struct for waiting for goals
typedef struct waiter
{
    bool active;
    pthread_mutex_t lock;
    pthread_cond_t cond;
} waiter;

// List of structs with cv's to signal on each goal
waiter waiters[2][MAX_SPECS];
// Data required by threads waiting for seat
typedef struct seat
{
    int type;
    int *seat_type;
    pthread_mutex_t *lock;
    pthread_cond_t *cond;
    struct timespec *end_wait;
} seat;

// Initialize stadium
void init_stad(void)
{
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < MAX_SPECS; j++)
        {
            waiters[i][j].active = false;
            pthread_mutex_init(&waiters[i][j].lock, NULL);
            pthread_cond_init(&waiters[i][j].cond, NULL);
        }
    }
}
// Thread that waits for a seat
void *seat_wait_sim(void *seat_arg)
{
    seat *s = (seat *)seat_arg;
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
// Spectator thread
void *spec_sim(void *spec_arg)
{
    spec *s = spec_arg;
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

    // Start threads waiting for seat
    pthread_mutex_lock(&seat_lock);
    if (s->type == A || s->type == N)
    {
        seat *a = malloc(sizeof(seat));
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
    // Check if any of the threads acquired a seat
    if (s->seat_type == E)
    {
        s->seat_type = X;
        pthread_mutex_unlock(&seat_lock);
        goto noseat;
    }
    pthread_mutex_unlock(&seat_lock);

    printf(GREEN "%s has got a seat in zone %c\n" RESET, s->name, zones[s->seat_type]);

    // Get lock and cond
    if (s->type == N)
    {
        sleep(spec_time);
        goto endspec;
    }
    pthread_mutex_t *scoreboard_lock = NULL;
    pthread_cond_t *scoreboard_cond = NULL;

    for (int i = 0; i < MAX_SPECS; i++)
    {
        if ((pthread_mutex_trylock(&waiters[s->type][i].lock) == 0) )
        {
            if (waiters[s->type][i].active == false)
            {
            waiters[s->type][i].active = true;
            scoreboard_lock = &waiters[s->type][i].lock;
            scoreboard_cond = &waiters[s->type][i].cond;
            break;
            }
            else
            {   
                pthread_mutex_unlock(&waiters[s->type][i].lock);
            }
        }
    }
    // Check if there is a waiter
    assert(scoreboard_lock != NULL);

    // Set endtime
    end_wait.tv_sec = time(NULL) + spec_time;
    // Wait for scoreboard
    while (scoreboard[(s->type + 1) % 2] < s->goals)
    {
        rt = pthread_cond_timedwait(scoreboard_cond, scoreboard_lock, &end_wait);
        if (rt != 0)
        {
            pthread_mutex_unlock(scoreboard_lock);
            goto endspec;
        }
    }
    printf(YELLOW "%s is leaving due to bad performance of his team\n" RESET, s->name);
    pthread_mutex_unlock(scoreboard_lock);

    // Release the seat
    sem_post(&seats[s->seat_type]);

    // Wait for friends at exit
    printf(CYAN "%s is waiting for their friends at the exit\n", s->name);
    pthread_exit(NULL);

noseat:;
    printf(YELLOW "%s couldn't get a seat\n" RESET, s->name);
    printf(CYAN "%s is waiting for their friends at the exit\n" RESET, s->name);
    pthread_exit(NULL);
endspec:;
    printf(YELLOW "%s watched the match for %d seconds and is leaving\n" RESET, s->name, spec_time);

    // Release the seat
    sem_post(&seats[s->seat_type]);

    // Wait for friends at exit
    printf(CYAN "%s is waiting for their friends at the exit\n" RESET, s->name);
    pthread_exit(NULL);
}

void *grp_sim(void *grp_arg)
{
    grp *g = (grp *)grp_arg;
    for (int i = 0; i < g->size; i++)
    {
        pthread_join(g->threads[i], NULL);
    }
    printf(MAGENTA "Group %d is leaving for dinner\n" RESET, g->id);
}

void match_sim(void)
{
    int chances;
    scanf("%d", &chances);
    int ctime = 0;
    char gteam;
    int gtime;
    double gprob;
    for (int i = 0; i < chances; i++)
    {
        scanf("\n%c %d %lf", &gteam, &gtime, &gprob);
        sleep(gtime - ctime);
        ctime = gtime;
        if (rand() % 100 < gprob * 100)
        {
            scoreboard[types[gteam]]++;
            printf("Team %c has scored their %d goal\n", gteam, scoreboard[types[gteam]]);
            for (int i = 0; i < MAX_SPECS;i++)
            {
                pthread_mutex_lock(&waiters[types[gteam]][i].lock);
                if (waiters[types[gteam]][i].active == true)
                {
                    pthread_cond_signal(&waiters[types[gteam]][i].cond);
                }
                pthread_mutex_unlock(&waiters[types[gteam]][i].lock);
            }
        }
        else
        {
            printf("Team %c has missed their %d goal\n", gteam, scoreboard[types[gteam]] + 1);
        }
    }
}
