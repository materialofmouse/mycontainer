#include <linux/capability.h>
#include <sys/capability.h>
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

cap_value_t read_cap_from_file() {
	FILE* file;
	file = fopen("/home/mouse/work/mycontainer/config/capabilities.conf", "r");
	if (file == NULL){
		perror("\x1b[31m[ERROR]\x1b[0m capabilities.conf is not found");
		return -1;
	}
	
	char line[256];
	while(fgets(line, 256, file) != NULL) {
		char* end;
		int cap;
		//capability section header
		if(line[0] == '[') {
			printf("c %s", line);
		}
		else if (line[0] == '\n') {
		}
		else {
			//str -> int
			cap = strtol(line, &end, 10);
			if (errno != 0){
				printf("\x1b[31m[ERROR]\x1b[0m CAP format is valid\n");
				return -1;
			}
			else if (end == line){
				printf("\x1b[31m[ERROR]\x1b[0m CAP is string\n");
				return -1;
			}
			else {
				if(CAP_AUDIT_CONTROL == 31){
					printf("line: %d\n", CAP_AUDIT_CONTROL);
				}
			}
		}
	}
	fclose(file);
	return 0;
}

#define CAP_COUNT 16
int set_capability() {
	//cap_t caps = cap_init();
	const cap_value_t cap_list[CAP_COUNT] = {
		CAP_CHOWN,
		CAP_NET_RAW,
		CAP_FOWNER,
		CAP_FSETID,
		CAP_KILL,
		CAP_SETGID,
		CAP_SETUID,
		CAP_SETPCAP,
		CAP_NET_RAW,
		CAP_SYS_CHROOT,
		CAP_SYS_ADMIN,
		CAP_MKNOD,
		CAP_AUDIT_WRITE,
		CAP_SETFCAP,
		CAP_DAC_OVERRIDE,
		CAP_DAC_READ_SEARCH,
	};

	cap_t caps = cap_get_proc();
		// -- debug --
	printf("current capability:%s\n",cap_to_text(caps, NULL));
	//printf("\n");

	if(cap_clear_flag(caps, CAP_INHERITABLE)){
		perror("\x1b[31m[ERROR]\x1b[0m cap_clear_flag");
	}

	if(cap_clear_flag(caps, CAP_PERMITTED)){
		perror("\x1b[31m[ERROR]\x1b[0m cap_clear_flag");	
	}
	if(cap_clear_flag(caps, CAP_EFFECTIVE)){
		perror("\x1b[31m[ERROR]\x1b[0m cap_clear_flag");
	}
//cap_set_flag(caps, CAP_PERMITTED, 1, flag, CAP_CLEAR);
	if(cap_set_flag(caps, CAP_PERMITTED, CAP_COUNT, cap_list, CAP_SET) == -1){ 
		perror("\x1b[31m[ERROR]\x1b[0m cap_set_flag prm");
		return -1;
	}
	if(cap_set_flag(caps, CAP_INHERITABLE, CAP_COUNT, cap_list, CAP_SET) == -1) {
		perror("\x1b[31m[ERROR]\x1b[0m cap_set_flag inh");
		return -1;
	}
	if(cap_set_flag(caps, CAP_EFFECTIVE, CAP_COUNT, cap_list, CAP_SET) == -1) {
		perror("\x1b[31m[ERROR]\x1b[0m cap_set_flag eff");
	}
	CAP_AMBIENT_SUPPORTED();
	int i;
	for(i = 0; i < CAP_COUNT; i++){
		cap_set_ambient(cap_list[i], CAP_SET);
	}
	if(cap_set_proc(caps) == -1) {
		perror("\x1b[31m[ERROR]\x1b[0m cap_set_proc");
		return -1;
	}
	// -- debug --
	caps = cap_get_proc();
	printf("\x1b[36m[DEBUG]\x1b[0m capability:%s\n", cap_to_text(caps, NULL));
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
	return 0;
}

void umount_container() {
	char pwd[256];
	getcwd(pwd, 256);
	char proc_path[512];
	char root_path[512];

	sprintf(proc_path, "%s/condir/root/proc", pwd);
	sprintf(root_path, "%s/condir/root", pwd);
	if(umount(proc_path) < 0 ) perror("\x1b[31m[ERROR]\x1b[0m umount");
	if(umount(root_path) < 0 ) perror("\x1b[31m[ERROR]\x1b[0m umount");
}


