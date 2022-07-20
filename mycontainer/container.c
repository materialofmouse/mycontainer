#define _GNU_SOURCE
#include <errno.h>
#include <sched.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <sys/mount.h>

#include "container.h"

int init_proc() {
	char* PROC_PATH = "/proc";
	if( access( PROC_PATH, F_OK) < 0){
		if( mkdir(PROC_PATH, 0555) < 0){
			perror("\x1b[31m[ERROR] cannot mkdir at proc path");
			return -1;
		}
	}
	if (mount("proc", "/proc", "proc", 0, NULL) < 0) {
				perror("\x1b[31m[ERROR] mount proc");
			return -1;
	}
	return 0;
}

int init_overlay(){ 
	char* ROOT_PATH = "/home/mouse/work/mycontainer/condir/overlay";
	errno = 0;
	if(mount("overlay", "/home/mouse/work/mycontainer/condir/root", "overlay", 0,
				"lowerdir=/home/mouse/work/mycontainer/debian,upperdir=/home/mouse/work/mycontainer/condir/root,workdir=/home/mouse/work/mycontainer/condir/work") != 0){ 
		perror("\x1b[31m[ERROR] mount overlay");
		return -1;
	}
	return 0;
}

int container_start() { 
		if(init_overlay() < 0) {
		perror("\x1b[31m[ERROR] init overlay");
		return -1;
	}

	printf("\x1b[36m[DEBUG]\x1b[0m child process:%d\n",(int)getpid());
	sethostname("container",9);
	if (chdir("/home/mouse/work/mycontainer/condir/root") < 0) {
		perror("\x1b[31m[ERROR] chdir");
		return -1;
	}
	if (chroot("/home/mouse/work/mycontainer/condir/root") < 0) {
		perror("\x1b[31m[ERROR] chroot");
		return -1;
	}
	if (init_proc() < 0){
		perror("\x1b[31m[ERROR] init_proc");
		return -1;
	}
	
	if (execl("/bin/bash","",NULL) < 0){
		perror("\x1b[31m[ERROR] bash");
		return -1;
	}
	return 0;
}


