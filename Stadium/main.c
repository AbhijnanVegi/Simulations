#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <semaphore.h>

#include "structs.h"
#include "sims.h"

sem_t seats[3];
int spec_time;

extern int scoreboard[2];
extern pthread_mutex_t scoreboard_lock;
extern pthread_cond_t scoreboard_cond[2];

int types[] = {
    ['H'] H,
    ['A'] A,
    ['N'] N
};

void match_sim(void)
{
    int chances;
    scanf("%d", &chances);
    int ctime = 0;
    char gteam;
    int gtime;
    double gprob;
    for (int i = 0; i < chances;i++)
    {
        scanf("\n%c %d %lf", &gteam, &gtime, &gprob);
        sleep(gtime-ctime);
        ctime = gtime;
        if (rand() % 100 < gprob * 100)
        {
            pthread_mutex_lock(&scoreboard_lock);
            scoreboard[types[gteam]]++;
            pthread_cond_broadcast(&scoreboard_cond[types[gteam]]);
            pthread_mutex_unlock(&scoreboard_lock);
            printf("Team %c has scored their %d goal\n", gteam, scoreboard[types[gteam]]);
        }
        else
        {
            printf("Team %c has missed their %d goal\n", gteam, scoreboard[types[gteam]]+1);
        }
    }
}

int main()
{
    srand(time(0));
    printf("🚀 Starting simulation\n");
    init_stad();
    int h, a, n;
    scanf("%d %d %d", &h, &a, &n);
    sem_init(&seats[H], 0, h);
    sem_init(&seats[A], 0, a);
    sem_init(&seats[N], 0, n);
    scanf("%d", &spec_time);

    int num_grps;
    scanf("%d", &num_grps);

    pthread_t grp_threads[num_grps];
    for (int i = 0; i < num_grps; i++)
    {
        grp *g = malloc(sizeof(grp));
        g->id = i + 1;
        scanf("%d", &g->size);
        g->threads = malloc(sizeof(pthread_t) * g->size);
        for (int j = 0; j < g->size; j++)
        {
            spec *s = malloc(sizeof(spec));
            char name[64];
            char type;

            scanf("%s %c %d %d %d", name, &type, &s->entry_time, &s->patience, &s->goals);
            s->name = strdup(name);
            s->type = types[type]; // H, A, or N
            s->seat_type = -1;
            pthread_t spec_thread;
            pthread_create(&spec_thread, NULL, spec_sim, s);
            g->threads[j] = spec_thread;
        }
        pthread_create(&grp_threads[i], NULL, grp_sim, g);
    }
    match_sim();
    for (int i = 0; i < num_grps; i++)
    {
        pthread_join(grp_threads[i], NULL);
    }
    printf("🎊 Simulation complete\n");
}