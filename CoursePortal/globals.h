#if !defined(CP_GLOBALS_H)
#define CP_GLOBALS_H

#include "structs.h"

extern lab *ilabs;
extern course *courses;
extern student *students;

extern int lab_c;
extern int course_c;
extern int student_c;

extern pthread_mutex_t stu_lock;

#endif // CP_GLOBALS_H
