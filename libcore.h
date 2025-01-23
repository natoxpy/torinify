#include <stdbool.h>

int cl_init();
void cl_cleanup();

void cl_set_audio_file(char *filename);
void cl_set_current_time(unsigned int miliseconds);

void cl_play();
void cl_pause();
bool cl_get_paused();
unsigned int cl_get_current_time();
unsigned int cl_get_duration();
float cl_get_volume();
void cl_set_volume(float volume);
