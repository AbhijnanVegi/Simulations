#if !defined(INIT_H)
#define INIT_H

void init_ta(struct ta *ta);
void init_lab(struct lab *lab, char *name, int ta_c, int ta_times);
void init_course(
    course *course,
    char *name,
    double interest,
    int max_slot_c,
    int lab_c,
    int *labs
);

void init_student(
    student *student,
    double calibre,
    int pref1,
    int pref2,
    int pref3,
    int apply_time
);

#endif // INIT_H
