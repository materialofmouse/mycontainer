#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>

#include "container.h"
#include "controller.h"
#include "cap_controller.h"

#define ERROR -1
#define DEBUG 0


void log_print(int _status, char* message) {
	char* status;
	if (_status == 0) status = "\x1b[36m[DEBUG]\x1b[0m";
	else if (_status == -1) status = "\x1b[31m[ERROR]\x1b[0m";
	else status = "[ETC]";

	printf("%s %s",status, message);
}

int main(){
	const unsigned int UNSHARE_FLAGS = ( CLONE_FILES | CLONE_NEWIPC | CLONE_NEWNS | CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWNET); 

	pid_t pid = getpid();
	errno = 0;
		//unshareでnamespaceと諸々を分ける
	if( unshare(UNSHARE_FLAGS) < 0){
		perror("[ERROR]: unshare");
		exit(1);
	}
	if (set_capability() < 0) {
		perror("\x1b[31m[ERROR]\x1b[0m set cap");
	}
	//fork
	read_cap_from_file();
	if( (pid = fork()) < 0) {
    perror("[ERROR]: fork");
    exit(1);
  }
	if( init_cgroup() < 0) {
		perror("[ERROR]: init_cgroup()\n");
		exit(1);
	}
	if( set_subsystem("+cpu") < 0) {
		perror("[ERROR]: set_subsystem()\n");
		exit(1);
	}
	if (pid == 0) {
		container_start();
	}
	else{
		write_pid(getpid(), pid);
		controller_start(&pid);
	}
	exit(0);
}
