#include <linux/capability.h>
#define _GNU_SOURCE
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<sys/capability.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<sched.h>
#include<sys/mount.h>
#include<fcntl.h>
#include<errno.h>


#include"child.h"
#include"parent.h"

void log_print(int _status, char* message) {
	char* status;
	if (_status == 0) status = "[DEBUG]";
	else if (_status == -1) status = "[ERROR]";
	else status = "[ETC]";
}

int main(){
	const unsigned int UNSHARE_FLAGS = ( CLONE_FILES | CLONE_NEWIPC | CLONE_NEWNS | CLONE_NEWUTS | CLONE_NEWPID); 

	pid_t pid = getpid();
	errno = 0;

	//unshareでnamespaceと諸々を分ける
	if( unshare(UNSHARE_FLAGS) < 0){
		perror("[ERROR]: unshare");
		exit(1);
	}
	//fork
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
	if (pid == 0) child_process();
	parrent_process(&pid);
	exit(0);
}
