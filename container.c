#define _GNU_SOURCE
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<unistd.h>
#include<stdio.h>
#include<sched.h>
#include<sys/mount.h>
#include<fcntl.h>
#include<errno.h>
#include</usr/include/linux/capability.h>

const unsigned int UNSHARE_FLAGS = ( CLONE_FILES | CLONE_NEWIPC | CLONE_NEWNS | CLONE_NEWUTS | CLONE_NEWPID); 


int main(){
	pid_t pid = getpid();
	errno = 0;

	int status;
	if( unshare(UNSHARE_FLAGS) < 0){
		perror("unshare");
		return 1;
	}
  if( (pid = fork()) < 0) {
    perror("fork");
    return 1;
  }
	//make cgroup
	if( access("/sys/fs/cgroup/container", F_OK) < 0){
		if( mkdir("/sys/fs/cgroup/container", 0644) < 0){
			perror("mkdir");
			return 1;
		}
	}

	int fd;
	//プロセスの登録
	fd = open("/sys/fs/cgroup/container/cgroup.procs", O_WRONLY);
	if( fd < 0){ perror("open"); return 1; }
	int _pid = getpid();
	char buff[6];
	snprintf(buff, 6 , "%d", _pid);
	write(fd, buff, 6);
	close(fd);
	
	//サブシステムの登録
	fd = open("/sys/fs/cgroup/cgroup.subtree_control", O_WRONLY);
	if (fd < 0){ perror("subtree error"); return 1;}
	write(fd, "+cpu", 5);
	close(fd);
	
	//CPU制限
	fd = open("/sys/fs/cgroup/container/cpu.max", O_WRONLY);
	if (fd < 0){ perror("cpu open"); return 1; }
	write(fd, "10000", 6);//このサーバーの場合
	close(fd);

	//child process
  if (pid == 0) {
		//capability
		cap_t caps;
		const cap_value_t cap_list[3] = {CAP_SYS_ADMIN, CAP_NET_RAW, CAP_SYS_CHROOT};

		if(cap_get_proc() == NULL) {
			printf("error:cap_get_proc\n");
		}
		if(cap_set_flag (caps, CAP_EFFECTIVE, 3, cap_list, CAP_SET) == -1) {
			printf("error:cap_set_flag\n");
		}
		if(cap_set_proc (caps) == -1) {
			printf("error:cap_set_proc\n");
		}

		printf("child process:%d\n",(int)getpid());
    sethostname("container",9);
		if (chdir("/home/mouse/container/debian") < 0) {
			perror("chdir");
			return 1;
		}
		if(chroot("/home/mouse/container/debian") < 0) {
			perror("chroot");
			return 1;
		}
		if (mount("proc", "/proc", "proc", 0, NULL) < 0) {
				perror("mount");
				return 1;
		}

		execl("/bin/bash","a",NULL);
    perror("bash");
  }

  printf("parrent process:%d\n",(int)getpid());
  if ((pid = waitpid(pid,&status,0)) < 0) {
    perror("wait");
    return 1;
  }
  if (WIFEXITED(status)) {
    printf("pid:%d status:%d\n",(int)getpid(),WEXITSTATUS(status));
  }
  return 0;
}
