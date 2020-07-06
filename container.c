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

int init_cgroup() {
	//cgroupの有効化
	if( access("/sys/fs/cgroup/container", F_OK) < 0){
		if( mkdir("/sys/fs/cgroup/container", 0644) < 0){
			perror("mkdir");
			return -1;
		}
	}

	int fd;
	//プロセスの登録
	fd = open("/sys/fs/cgroup/container/cgroup.procs", O_WRONLY);
	if( fd < 0){ perror("open"); return -1; }
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
		perror("subtree error"); 
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
		perror("cpu open"); 
		return -1; 
	}
	write(fd, "10000", 6);//このサーバーの場合
	close(fd);
	return 0;
}
//OverlayFSの使用にmountを行い、引数で設定する箇所があるためそれを行う関数
int overlayFS_init() { 
	return 0;
}

int child_process() { 
	cap_t caps = cap_init();
	const cap_value_t cap_list[14] = {CAP_SETPCAP,CAP_MKNOD,CAP_AUDIT_WRITE,CAP_CHOWN,CAP_NET_RAW,CAP_DAC_OVERRIDE,CAP_FOWNER,CAP_FSETID,CAP_KILL,CAP_SETGID,CAP_SETUID,CAP_NET_BIND_SERVICE,CAP_SYS_CHROOT,CAP_SETFCAP};

	caps = cap_get_proc();
	cap_clear(caps);
	if(cap_set_proc(caps) == -1) {
		printf("error:cap_set_proc\n");
		return -1;
	}
	printf("capability:%s\n",cap_to_text(caps, NULL));
	printf("\n");
	if(caps == NULL) printf("error:cap_get_proc\n");
	if(cap_set_flag(caps, CAP_PERMITTED, 14, cap_list, CAP_SET) == -1){ 
		printf("error:per\n");
		return -1;
	}
	if(cap_set_flag(caps, CAP_INHERITABLE, 14, cap_list, CAP_SET) == -1) {
		printf("error:inh\n");
		return -1;
	}
	if(cap_set_flag(caps, CAP_EFFECTIVE, 14, cap_list, CAP_SET) == -1) {
		printf("error:eff\n");
		return -1;
	}
	if(cap_set_proc(caps) == -1) {
		printf("error:cap_set_proc2\n");
		return -1;
	}
	caps = cap_get_proc();
	printf("capability:%s\n", cap_to_text(caps, NULL));
	if(cap_set_proc(caps) == -1) {
		printf("error:cap_set_proc\n");
		return -1;
	}

	printf("child process:%d\n",(int)getpid());
	sethostname("container",9);
	if (chdir("/home/mouse/container/debian") < 0) {
		perror("chdir");
		return -1;
	}
	if(chroot("/home/mouse/container/debian") < 0) {
		perror("chroot");
		return -1;
	}
	if (mount("proc", "/proc", "proc", 0, NULL) < 0) {
			perror("mount");
		return -1;
	}

	execl("/bin/bash","a",NULL);
	perror("bash");
}

int parrent_process(pid_t * pid) {
	int status;
	printf("parrent process:%d\n",(int)getpid());
	if ((*pid = waitpid(*pid,&status,0)) < 0) {
		perror("wait");
		return -1;
	}
	if (WIFEXITED(status)) {
		printf("pid:%d status:%d\n",(int)getpid(),WEXITSTATUS(status));
	}
}

int main(){
	const unsigned int UNSHARE_FLAGS = ( CLONE_FILES | CLONE_NEWIPC | CLONE_NEWNS | CLONE_NEWUTS | CLONE_NEWPID); 

	pid_t pid = getpid();
	errno = 0;

	//unshareでnamespaceと諸々を分ける
	if( unshare(UNSHARE_FLAGS) < 0){
		perror("unshare");
		exit(1);
	}
	//fork
  if( (pid = fork()) < 0) {
    perror("fork");
    exit(1);
  }
	if( init_cgroup() < 0) {
		printf("init_cgroup()\n");
		exit(1);
	}
	if( set_subsystem("+cpu") < 0) {
		printf("set_subsystem()\n");
		exit(1);
	}
	if (pid == 0) child_process();
	parrent_process(&pid);
	exit(0);
}
