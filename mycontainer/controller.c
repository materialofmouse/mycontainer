#include <sys/capability.h>
#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <stdio.h>
#include <sys/mount.h>

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

int set_capability() {
	cap_t caps = cap_init();
	const cap_value_t cap_list[15] = {
		CAP_SETPCAP,
		CAP_MKNOD,
		CAP_AUDIT_WRITE,
		CAP_CHOWN,
		CAP_NET_RAW,
		CAP_DAC_OVERRIDE,
		CAP_FOWNER,
		CAP_FSETID,
		CAP_KILL,
		CAP_SETGID,
		CAP_SETUID,
		CAP_NET_BIND_SERVICE,
		CAP_SYS_CHROOT,
		CAP_SETFCAP,
		CAP_SYS_ADMIN
	};

	caps = cap_get_proc();
	//cap_clear(caps);
	if(cap_set_proc(caps) == -1) {
		perror("\x1b[31m[ERROR]\x1b[0m cap_set_proc");
		return -1;
	}
	// -- debug --
	//printf("capability:%s\n",cap_to_text(caps, NULL));
	//printf("\n");

	if(cap_set_flag(caps, CAP_PERMITTED, 14, cap_list, CAP_SET) == -1){ 
		perror("\x1b[31m[ERROR]\x1b[0m cap_set_flag");
		return -1;
	}
	if(cap_set_flag(caps, CAP_INHERITABLE, 14, cap_list, CAP_SET) == -1) {
		return -1;
	}
	if(cap_set_flag(caps, CAP_EFFECTIVE, 14, cap_list, CAP_SET) == -1) {
		perror("\x1b[31m[ERROR]\x1b[0m cap_set_flag");
		return -1;
	}
	if(cap_set_proc(caps) == -1) {
		perror("\x1b[31m[ERROR]\x1b[0m cap_set_proc");
		return -1;
	}
	// -- debug --
	//caps = cap_get_proc();
	//printf("capability:%s\n", cap_to_text(caps, NULL));
	return 0;
}

int controller_start(pid_t * pid) {
	int status;
	printf("\x1b[36m[DEBUG]\x1b[0m controller process:%d\n",(int)getpid());
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
	char pwd[512];
	getcwd(pwd, 512);
	char proc_path[512];
	char root_path[512];

	sprintf(proc_path, "%s%s", pwd, "/condir/root/proc");
	sprintf(root_path, "%s%s", pwd, "/condir/root");
	if(umount(proc_path) < 0 ) perror("\x1b[31m[ERROR]\x1b[0m umount");
	if(umount(root_path) < 0 ) perror("\x1b[31m[ERROR]\x1b[0m umount");
}


