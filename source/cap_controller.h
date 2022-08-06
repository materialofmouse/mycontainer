#ifndef CAPABILITY_H
#define CAPABILIT_H

#include <linux/capability.h>
#include <sys/capability.h>
#include <sys/types.h>

cap_value_t read_cap_from_file();
int set_capability();

#endif
