#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <sys/types.h>
#include <linux/capability.h>
#include <sys/capability.h>

int init_cgroup();
int set_subsystem(char *subsytem);
int restrict_cpu(int percent);
cap_value_t read_cap_from_file();
int set_capability();
int controller_start(pid_t* pid);
void write_pid(pid_t ctl_pid, pid_t con_pid);
void umount_container();

#endif
