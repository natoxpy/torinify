
#ifndef _TORINIFY_CORE_H
#define _TORINIFY_CORE_H
#include <stdbool.h>

int cl_init(void);
void cl_cleanup(void);

int cl_set_audio_file(char *filename);
void cl_set_current_time(unsigned int miliseconds);

void cl_play(void);
void cl_pause(void);
bool cl_get_paused(void);
unsigned int cl_get_current_time(void);
unsigned int cl_get_duration(void);
float cl_get_volume(void);
void cl_set_volume(float volume);

#endif
