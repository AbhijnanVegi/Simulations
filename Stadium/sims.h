#if !defined(STD_SIMS_H)
#define STD_SIMS_H

void init_stad(void);
void *seat_wait(void *sleep_arg);
void *spec_wait(void *sleep_arg);
void *spec_sim(void *spec_arg);
void *grp_sim(void *grp_arg);
#endif // STD_SIMS_H
