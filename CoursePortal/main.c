#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "globals.h"
#include "init.h"
#include "sims.h"

lab *ilabs;
course *courses;
student *students;

int lab_c;
int course_c;
int student_c;

pthread_mutex_t stu_lock;

void init_glob()
{
    scanf("%d %d %d", &student_c, &lab_c, &course_c);
    ilabs = (lab *)malloc(sizeof(lab) * lab_c);
    courses = (course *)malloc(sizeof(course) * course_c);
    students = (student *)malloc(sizeof(student) * student_c);

    for (int i = 0; i < course_c; i++)
    {
        char name[128];
        double interest;
        int max_slot_c;
        int lab_c;
        scanf("%s %lf %d %d", name, &interest, &max_slot_c, &lab_c);
        int *lab_ids = (int *)malloc(sizeof(int) * lab_c);
        for (int j = 0; j < lab_c; j++)
        {
            scanf("%d", &lab_ids[j]);
        }
        init_course(&courses[i], name, interest, max_slot_c, lab_c, lab_ids);
    }

    for (int i = 0; i < student_c; i++)
    {
        double calibre;
        int *prefs = (int *)malloc(sizeof(int) * 3);
        int apply_time;
        scanf("%lf %d %d %d %d", &calibre, &prefs[0], &prefs[1], &prefs[2], &apply_time);
        init_student(&students[i], calibre, prefs, apply_time);
    }

    pthread_mutex_init(&stu_lock, NULL);

    for (int i = 0; i < lab_c; i++)
    {
        char name[128];
        int ta_c, ta_times;

        scanf("%s %d %d", name, &ta_c, &ta_times);
        init_lab(&ilabs[i], name, ta_c, ta_times);
    }
}

void start_sim()
{
    pthread_t stu_threads[student_c];
    // Start students sims
    for (int i = 0; i < student_c; i++)
    {
        pthread_create(&stu_threads[i], NULL, student_sim, &students[i]);
    }
    
    pthread_t course_threads[course_c];
    // Start course sims
    for (int i = 0; i < course_c; i++)
    {
        pthread_create(&course_threads[i], NULL, course_sim, &courses[i]);
    }
    pthread_t lab_threads[lab_c];
    // Start lab sims
    for (int i = 0; i < lab_c; i++)
    {
        pthread_create(&lab_threads[i], NULL, lab_sim, &ilabs[i]);
    }

    // Wait for all threads to finish
    for (int i = 0; i < student_c; i++)
    {
        pthread_join(stu_threads[i], NULL);
    }
    printf("All students have exited the simulation\n");
    for (int i = 0; i < course_c; i++)
    {
        pthread_cancel(course_threads[i]);
    }
    for(int i = 0; i < lab_c; i++)
    {
        pthread_cancel(lab_threads[i]);
    }
}

int main()
{
    srand(time(0));
    init_glob();
    printf("🚀 Starting simulation\n");
    start_sim();
    printf("🎊 Simulation complete\n");

    return 0;
}