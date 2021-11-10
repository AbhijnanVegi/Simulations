#include <stdio.h>
#include <stdlib.h>

#include "globals.h"
#include "init.h"

lab *ilabs;
course *courses;
student *students;

int lab_c;
int course_c;
int student_c;

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
        int pref1, pref2, pref3;
        int apply_time;
        scanf("%lf %d %d %d %d", &calibre, &pref1, &pref2, &pref3, &apply_time);
        init_student(&students[i], calibre, pref1, pref2, pref3, apply_time);
    }

    for (int i = 0; i < lab_c;i++)
    {
        char name[128];
        int ta_c, ta_times;

        scanf("%s %d %d", name, &ta_c, &ta_times);
        init_lab(&ilabs[i], name, ta_c, ta_times);
    }
}

int main()
{
    init_glob();
    printf("Globals initialised\n");
    return 0;
}