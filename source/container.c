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

#define MAX_PATH_LENGTH 128
static char current_path[MAX_PATH_LENGTH];

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
	char root_dir[MAX_PATH_LENGTH*2];
	char lower_dir[MAX_PATH_LENGTH*2];
	char upper_dir[MAX_PATH_LENGTH*2];
	char work_dir[MAX_PATH_LENGTH*2];
	char mount_option[MAX_PATH_LENGTH*6+1];

	sprintf(root_dir,  "%s/layer/root", current_path);
	sprintf(lower_dir, "%s/debian", current_path);
	sprintf(upper_dir, "%s/layer/diff", current_path);
	sprintf(work_dir,  "%s/layer/work", current_path);
	sprintf(mount_option, "lowerdir=%s,upperdir=%s,workdir=%s", lower_dir, upper_dir, work_dir);
	
	if(mount("overlay", root_dir, "overlay", 0, mount_option) != 0){ 
		perror("\x1b[31m[ERROR] mount overlay");
		return -1;
	}
	return 0;
}

int container_start() {
	getcwd(current_path, MAX_PATH_LENGTH); 
	
	if(init_overlay() < 0) {
		perror("\x1b[31m[ERROR] init overlay");
		return -1;
	}

	printf("\x1b[36m[DEBUG]\x1b[0m child process:%d\n",(int)getpid());
	sethostname("container",9);
	
	char root_dir[MAX_PATH_LENGTH*2];
	sprintf(root_dir, "%s/layer/root", current_path);
	if (chdir(root_dir) < 0) {
		perror("\x1b[31m[ERROR] chdir");
		return -1;
	}
	if (chroot(root_dir) < 0) {
		perror("\x1b[31m[ERROR] chroot");
		return -1;
	}
	if (init_proc() < 0){
		perror("\x1b[31m[ERROR] init_proc");
		return -1;
	}
	
	if (execl("./start.sh", "", NULL) < 0){
		perror("\x1b[31m[ERROR] bash");
		return -1;
	}
	return 0;
}
