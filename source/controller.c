#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/mount.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "controller.h"

int init_cgroup() {
	char* CGROUP_PATH = "/sys/fs/cgroup/container";
	//cgroupの有効化
	if( access( CGROUP_PATH, F_OK) < 0){
		if( mkdir(CGROUP_PATH, 0644) < 0){
			perror("\x1b[31m[ERROR]\x1b[0m cannot mkdir at cgroup path");
			return -1;
		}
	}
	else return 0;

	int fd;
	//プロセスの登録
	fd = open("/sys/fs/cgroup/container/cgroup.procs", O_WRONLY);
	if( fd < 0 ){ 
		perror("\x1b[31m[ERROR]\x1b[0m fd cannot open"); 
		return -1; 
	}
	int _pid = getpid();
	char buff[6];
	snprintf(buff, 6 , "%d", _pid);
	write(fd, buff, 6);
	close(fd);
	return 0;
}

int set_subsystem(char *subsytem) {
//サブシステムの登録
	int fd;
	fd = open("/sys/fs/cgroup/cgroup.subtree_control", O_WRONLY);
	if (fd < 0) { 
		perror("\x1b[31m[ERROR]\x1b[0m subtree error"); 
		return -1;
	}
	write(fd, "+cpu", 5);
	close(fd);
	return 0;
}

int restrict_cpu(int percent) {
	int fd;
	fd = open("/sys/fs/cgroup/container/cpu.max", O_WRONLY);
	if (fd < 0){ 
		perror("\x1b[31m[ERROR]\x1b[0m cpu open"); 
		return -1; 
	}
	write(fd, "10000", 6);//このサーバーの場合
	close(fd);
	return 0;
}


int controller_start(pid_t * pid) {
	int status;
	int _pid = getpid();
	char mypid[6];
	sprintf(mypid, "%d", _pid);
	printf("\x1b[36m[DEBUG]\x1b[0m controller process:%d\n",_pid);

//set_capability();
	if ((*pid = waitpid(*pid,&status,0)) < 0) {
		perror("\x1b[31m[ERROR]\x1b[0m wait");
		return -1;
	}
	if (WIFEXITED(status)) {
		printf("\x1b[36m[DEBUG]\x1b[0m pid:%d status:%d\n",(int)getpid(),WEXITSTATUS(status));
		umount_container();
	}
	write_pid(0,0);
	return 0;
}

void write_pid(pid_t ctl_pid, pid_t con_pid){
	FILE *ctl_pid_f;
	FILE *con_pid_f;
	
	ctl_pid_f = fopen("./status/controller_pid", "w+");
	con_pid_f = fopen("./status/container_pid", "w+");
	fprintf(ctl_pid_f, "%d\n", ctl_pid);
	fprintf(con_pid_f, "%d\n", con_pid);
	fclose(con_pid_f);
	fclose(ctl_pid_f);
}

void umount_container() {
	char pwd[256];
	getcwd(pwd, 256);
	char proc_path[512];
	char root_path[512];

	sprintf(proc_path, "%s/layer/root/proc", pwd);
	sprintf(root_path, "%s/layer/root", pwd);
	if(umount(proc_path) < 0 ) perror("\x1b[31m[ERROR]\x1b[0m umount");
	if(umount(root_path) < 0 ) perror("\x1b[31m[ERROR]\x1b[0m umount");
}


