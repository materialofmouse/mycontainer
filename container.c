#define _GNU_SOURCE
#define _LINUX_CAPABILITY_VERSION_3 0x20080522
#define _LINUX_CAPABILITY_U32S_2 2
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<unistd.h>
#include<stdio.h>
#include<sched.h>
#include<sys/mount.h>
#include<fcntl.h>
#include</usr/include/linux/capability.h>

typedef struct __user_cap_header_struct {
	__u32 version;
	int pid;
} *cap_user_header_t;

typedef struct __user_cap_data_struct {
	__u32 effective;
	__u32 permitted;
	__u32 inheritab;
} *cap_user_data_t;


const unsigned int UNSHARE_FLAGS = ( CLONE_FILES | CLONE_NEWIPC | CLONE_NEWNS | CLONE_NEWUTS | CLONE_NEWPID); 


int main(){
  pid_t pid;
  int status;
	if( unshare(UNSHARE_FLAGS) == -1){
		perror("unshare");
		return 1;
	}
  if( (pid = fork()) < 0) {
    perror("fork");
    return 1;
  }
	//make cgroup
	if( access("/sys/fs/cgroup/container", F_OK) != 0){
		if( mkdir("/sys/fs/cgroup/container", 0644) != 0){
			perror("mkdir");
			return 1;
		}
	}

	int fd;
	//プロセスの登録
	fd = open("/sys/fs/cgroup/container/cgroup.procs", O_WRONLY);
	if( fd == -1){ perror("open"); return 1; }
	int _pid = getpid();
	char buff[6];
	snprintf(buff, 6 , "%d", _pid);
	write(fd, buff, 6);
	close(fd);
	
	//サブシステムの登録
	fd = open("/sys/fs/cgroup/cgroup.subtree_control", O_WRONLY);
	if (fd == -1){ perror("subtree error"); return 1;}
	write(fd, "+cpu", 5);
	close(fd);
	
	//CPU制限
	fd = open("/sys/fs/cgroup/container/cpu.max", O_WRONLY);
	if (fd == -1){ perror("cpu open"); return 1; }
	write(fd, "10000", 6);//このサーバーの場合
	close(fd);

	//capability header
	cap_user_header_t.version = 3;
	cap_user_header_t.pid = _pid;

	//capability data

	//child process
  if (pid == 0) {
    printf("child process:%d\n",(int)getpid());
    sethostname("container",9);
		if (chdir("/home/mouse/container/debian") != 0){
			perror("chdir");
			return 1;
		}
		if(chroot("/home/mouse/container/debian") != 0){
			perror("chroot");
			return 1;
		}
		if (mount("proc", "/proc", "proc", 0, NULL) != 0){
				perror("mount");
				return 1;
		}

		execl("/bin/bash","a",NULL);
    perror("bash");
    //_exit(1);
  }

  printf("parrent proces:%d\n",(int)getpid());
  if ((pid = waitpid(pid,&status,0)) < 0) {
    perror("wait");
    return 1;
  }
  if (WIFEXITED(status)) {
    printf("pid:%d status:%d\n",(int)getpid(),WEXITSTATUS(status));
  }
  return 0;
}
