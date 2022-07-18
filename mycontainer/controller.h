#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <sys/types.h>

int init_cgroup();
int set_subsystem(char *subsytem);
int restrict_cpu(int percent);
int set_capability();
int controller_start(pid_t* pid);
void umount_container();

#endif
